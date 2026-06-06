/**
 * @file SignalSynchronizer.cpp
 * @brief SignalSynchronizer 实现
 *
 * 实现信号同步：互相关峰值检测、抛物线分数延迟估计、频域FFT同步、sinc插值对齐。
 */

#include "utils/signal169/SignalSynchronizer.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

SignalSynchronizer::SignalSynchronizer(QObject* parent)
    : QObject(parent)
{
}

SignalSynchronizer::~SignalSynchronizer() = default;

void SignalSynchronizer::setMaxDelay(int maxDelay)
{
    m_maxDelay = qMax(1, maxDelay);
}

double SignalSynchronizer::parabolicInterp(double ym1, double y0, double yp1) const
{
    double denom = ym1 - 2.0 * y0 + yp1;
    if (qAbs(denom) < 1e-15) return 0.0;
    return (ym1 - yp1) / (2.0 * denom);
}

void SignalSynchronizer::fft(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    int log2n = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; log2n++; }

    for (int i = 0; i < n; ++i) {
        int rev = 0, x = i;
        for (int b = 0; b < log2n; ++b) { rev = (rev << 1) | (x & 1); x >>= 1; }
        if (rev > i) { std::swap(real[i], real[rev]); std::swap(imag[i], imag[rev]); }
    }

    for (int step = 1; step < n; step <<= 1) {
        double angle = M_PI / step;
        double wR = qCos(angle), wI = -qSin(angle);
        for (int i = 0; i < n; i += (step << 1)) {
            double cR = 1.0, cI = 0.0;
            for (int j = i; j < i + step; ++j) {
                double tR = cR * real[j + step] - cI * imag[j + step];
                double tI = cR * imag[j + step] + cI * real[j + step];
                real[j + step] = real[j] - tR;
                imag[j + step] = imag[j] - tI;
                real[j] += tR;
                imag[j] += tI;
                double nr = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nr;
            }
        }
    }
}

void SignalSynchronizer::ifft(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    for (int i = 0; i < n; ++i) imag[i] = -imag[i];
    fft(real, imag);
    for (int i = 0; i < n; ++i) { real[i] /= n; imag[i] = -imag[i] / n; }
}

double SignalSynchronizer::sinc(double x)
{
    if (qAbs(x) < 1e-10) return 1.0;
    double px = M_PI * x;
    return qSin(px) / px;
}

QVector<double> SignalSynchronizer::crossCorrelate(const QVector<double>& a,
                                                     const QVector<double>& b) const
{
    int na = a.size(), nb = b.size();
    int len = na + nb - 1;
    QVector<double> corr(len, 0.0);
    for (int lag = -(nb - 1); lag < na; ++lag) {
        double sum = 0.0;
        for (int j = 0; j < nb; ++j) {
            int idx = lag + j;
            if (idx >= 0 && idx < na)
                sum += a[idx] * b[j];
        }
        corr[lag + nb - 1] = sum;
    }
    return corr;
}

SignalSynchronizer::SyncResult SignalSynchronizer::synchronize(
    const QVector<double>& reference, const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    SyncResult result;
    int n = qMin(reference.size(), signal.size());
    if (n == 0) return result;

    m_stats.lastSignalLength = n;

    /* Time-domain cross-correlation within search range */
    int maxLag = qMin(m_maxDelay, n / 2);
    double bestCorr = -std::numeric_limits<double>::max();
    int bestLag = 0;

    /* Normalize energies */
    double refEnergy = 0.0;
    for (int i = 0; i < n; ++i) refEnergy += reference[i] * reference[i];
    refEnergy = qSqrt(qMax(refEnergy, 1e-15));

    for (int lag = -maxLag; lag <= maxLag; ++lag) {
        double corr = 0.0;
        double sigEnergy = 0.0;
        for (int i = 0; i < n; ++i) {
            int si = i - lag;
            if (si >= 0 && si < n) {
                corr += reference[i] * signal[si];
                sigEnergy += signal[si] * signal[si];
            }
        }
        sigEnergy = qSqrt(qMax(sigEnergy, 1e-15));
        corr /= (refEnergy * sigEnergy);

        if (corr > bestCorr) {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    result.integerDelay = bestLag;
    result.correlationPeak = bestCorr;

    /* Fractional delay via parabolic interpolation */
    if (bestLag > -maxLag && bestLag < maxLag) {
        auto corrAt = [&](int lag) -> double {
            double c = 0.0;
            for (int i = 0; i < n; ++i) {
                int si = i - lag;
                if (si >= 0 && si < n) c += reference[i] * signal[si];
            }
            return c;
        };
        double ym1 = corrAt(bestLag - 1);
        double y0 = bestCorr;
        double yp1 = corrAt(bestLag + 1);
        result.fractionalDelay = parabolicInterp(ym1, y0, yp1);
    }

    result.delaySamples = bestLag + result.fractionalDelay;

    /* SNR estimate from peak correlation */
    result.snrEstimate = (bestCorr > 1e-10)
        ? 20.0 * qLn(bestCorr / qMax(1.0 - bestCorr, 1e-10)) / M_LN2
        : 0.0;

    m_stats.totalSyncs++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSyncs > 0)
        ? m_timeSum / m_stats.totalSyncs : 0.0;

    emit syncCompleted(result.delaySamples);
    return result;
}

SignalSynchronizer::SyncResult SignalSynchronizer::synchronizeFFT(
    const QVector<double>& reference, const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    SyncResult result;
    int n = qMin(reference.size(), signal.size());
    if (n == 0) return result;

    int fftSize = 1;
    while (fftSize < 2 * n) fftSize <<= 1;

    /* Zero-pad and FFT both signals */
    QVector<double> refR(fftSize, 0.0), refI(fftSize, 0.0);
    QVector<double> sigR(fftSize, 0.0), sigI(fftSize, 0.0);
    for (int i = 0; i < n; ++i) { refR[i] = reference[i]; sigR[i] = signal[i]; }

    fft(refR, refI);
    fft(sigR, sigI);

    /* Cross-spectrum: R(f) * conj(S(f)) */
    QVector<double> crossR(fftSize), crossI(fftSize);
    for (int i = 0; i < fftSize; ++i) {
        crossR[i] = refR[i] * sigR[i] + refI[i] * sigI[i];
        crossI[i] = refI[i] * sigR[i] - refR[i] * sigI[i];
    }

    ifft(crossR, crossI);

    /* Find peak in cross-correlation */
    double bestVal = -std::numeric_limits<double>::max();
    int bestIdx = 0;
    for (int i = 0; i < fftSize; ++i) {
        double val = crossR[i];
        if (val > bestVal) { bestVal = val; bestIdx = i; }
    }

    /* Handle wrap-around */
    int lag = (bestIdx <= fftSize / 2) ? bestIdx : bestIdx - fftSize;
    result.integerDelay = lag;
    result.correlationPeak = bestVal / n;

    /* Fractional delay */
    int prev = (bestIdx - 1 + fftSize) % fftSize;
    int next = (bestIdx + 1) % fftSize;
    result.fractionalDelay = parabolicInterp(crossR[prev], crossR[bestIdx], crossR[next]);
    result.delaySamples = lag + result.fractionalDelay;
    result.snrEstimate = 20.0 * qLn(qMax(bestVal, 1e-10) / n) / M_LN2;

    m_stats.totalSyncs++;
    m_stats.lastSignalLength = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSyncs > 0)
        ? m_timeSum / m_stats.totalSyncs : 0.0;

    emit syncCompleted(result.delaySamples);
    return result;
}

QVector<double> SignalSynchronizer::alignSignal(const QVector<double>& signal,
                                                  double delay)
{
    int n = signal.size();
    QVector<double> aligned(n, 0.0);
    int intDelay = static_cast<int>(qRound(delay));
    for (int i = 0; i < n; ++i) {
        int src = i - intDelay;
        if (src >= 0 && src < n)
            aligned[i] = signal[src];
    }
    return aligned;
}

void SignalSynchronizer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
