/**
 * @file SignalSynchronizer6.cpp
 * @brief SignalSynchronizer6 实现
 *
 * 实现信号同步器：交叉谱相位估计、分数延迟插值与时间对齐。
 */

#include "utils/signal241/SignalSynchronizer6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SignalSynchronizer6::SignalSynchronizer6(QObject *parent) : QObject(parent) {}
SignalSynchronizer6::~SignalSynchronizer6() = default;

/* ---- Configuration ---- */

void SignalSynchronizer6::setSampleRate(double hz) { m_sampleRate = qBound(1.0, hz, 192000.0); }
void SignalSynchronizer6::setFftSize(int size)
{
    int s = 64;
    while (s < size) s *= 2;
    m_fftSize = s;
}

/* ---- Windowed FFT ---- */

void SignalSynchronizer6::computeFFT(const QVector<double>& input,
                                     QVector<double>& re, QVector<double>& im) const
{
    int N = m_fftSize;
    re.resize(N, 0.0);
    im.resize(N, 0.0);

    // Apply Hann window and zero-pad
    int len = qMin(input.size(), N);
    for (int i = 0; i < len; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / len));
        re[i] = input[i] * w;
    }

    // Radix-2 FFT
    int log2n = 0, tmp = N;
    while (tmp > 1) { tmp >>= 1; log2n++; }

    // Bit reversal
    for (int i = 0; i < N; ++i) {
        int rev = 0, val = i;
        for (int b = 0; b < log2n; ++b) { rev = (rev << 1) | (val & 1); val >>= 1; }
        if (rev > i) {
            std::swap(re[i], re[rev]);
            std::swap(im[i], im[rev]);
        }
    }

    // Butterfly
    for (int len2 = 2; len2 <= N; len2 *= 2) {
        int half = len2 / 2;
        for (int i = 0; i < N; i += len2) {
            for (int j = 0; j < half; ++j) {
                double angle = -2.0 * M_PI * j / len2;
                double cosA = qCos(angle), sinA = qSin(angle);
                double tRe = cosA * re[i + j + half] - sinA * im[i + j + half];
                double tIm = sinA * re[i + j + half] + cosA * im[i + j + half];
                re[i + j + half] = re[i + j] - tRe;
                im[i + j + half] = im[i + j] - tIm;
                re[i + j] += tRe;
                im[i + j] += tIm;
            }
        }
    }
}

/* ---- Cross-spectrum and coherence ---- */

void SignalSynchronizer6::crossSpectrum(const QVector<double>& refRe, const QVector<double>& refIm,
                                        const QVector<double>& sigRe, const QVector<double>& sigIm,
                                        QVector<double>& crossRe, QVector<double>& crossIm,
                                        QVector<double>& coherence) const
{
    int N = refRe.size();
    crossRe.resize(N);
    crossIm.resize(N);
    coherence.resize(N);

    QVector<double> refPow(N), sigPow(N);

    for (int k = 0; k < N; ++k) {
        // Cross power: conj(Ref) * Sig
        crossRe[k] = refRe[k] * sigRe[k] + refIm[k] * sigIm[k];
        crossIm[k] = refIm[k] * sigRe[k] - refRe[k] * sigIm[k];

        refPow[k] = refRe[k] * refRe[k] + refIm[k] * refIm[k];
        sigPow[k] = sigRe[k] * sigRe[k] + sigIm[k] * sigIm[k];

        double crossMag = qSqrt(crossRe[k] * crossRe[k] + crossIm[k] * crossIm[k]);
        double denom = qSqrt(refPow[k] * sigPow[k]);
        coherence[k] = (denom > 1e-15) ? crossMag / denom : 0.0;
    }
}

/* ---- Lanczos kernel ---- */

double SignalSynchronizer6::lanczosKernel(double x, int a) const
{
    if (qFuzzyCompare(x, 0.0)) return 1.0;
    if (x <= -a || x >= a) return 0.0;
    double pix = M_PI * x;
    return (a * qSin(pix) * qSin(pix / a)) / (pix * pix);
}

/* ---- Sinc interpolation ---- */

double SignalSynchronizer6::sincInterpolate(const QVector<double>& signal, double index) const
{
    int a = 4;  // Lanczos parameter
    int n = signal.size();
    int baseIdx = qFloor(index);
    double frac = index - baseIdx;

    double sum = 0.0;
    for (int i = baseIdx - a + 1; i <= baseIdx + a; ++i) {
        if (i < 0 || i >= n) continue;
        double x = static_cast<double>(i) - index;
        sum += signal[i] * lanczosKernel(x, a);
    }
    return sum;
}

/* ---- Estimate delay ---- */

SignalSynchronizer6::SyncResult SignalSynchronizer6::estimateDelay(
    const QVector<double>& reference, const QVector<double>& delayed)
{
    QElapsedTimer timer;
    timer.start();

    SyncResult result;

    // Compute FFTs
    QVector<double> refRe, refIm, sigRe, sigIm;
    computeFFT(reference, refRe, refIm);
    computeFFT(delayed, sigRe, sigIm);

    // Cross-spectrum
    QVector<double> crossRe, crossIm, coh;
    crossSpectrum(refRe, refIm, sigRe, sigIm, crossRe, crossIm, coh);

    // Weighted phase-based delay estimation
    double numSum = 0.0, denSum = 0.0;
    int halfN = m_fftSize / 2;

    for (int k = 1; k < halfN; ++k) {
        double phase = qAtan2(crossIm[k], crossRe[k]);
        double w = coh[k] * coh[k];  // coherence squared weighting
        double freq = static_cast<double>(k) / m_fftSize;
        // phase = -2*pi*freq*delay => delay = -phase / (2*pi*freq)
        numSum += -phase / (2.0 * M_PI * freq) * w;
        denSum += w;
    }

    double coarseDelay = (denSum > 1e-15) ? numSum / denSum : 0.0;

    // Cross-correlation refinement for integer part
    int maxLag = qMin(256, qMin(reference.size(), delayed.size()));
    double maxCorr = -1e30;
    int bestLag = 0;
    for (int lag = -maxLag; lag <= maxLag; ++lag) {
        double corr = 0.0;
        for (int i = 0; i < reference.size(); ++i) {
            int j = i + lag;
            if (j >= 0 && j < delayed.size())
                corr += reference[i] * delayed[j];
        }
        if (corr > maxCorr) { maxCorr = corr; bestLag = lag; }
    }

    // Combine: use cross-correlation integer lag + spectral fractional part
    double fracPart = coarseDelay - qRound(coarseDelay);
    result.delaySamples = bestLag + fracPart;
    result.timeOffsetMs = result.delaySamples / m_sampleRate * 1000.0;

    // Average coherence
    double cohSum = 0.0;
    for (int k = 0; k < halfN; ++k) cohSum += coh[k];
    result.coherence = cohSum / halfN;
    result.confidence = qBound(0.0, result.coherence, 1.0);

    m_stats.blockSize = qMax(reference.size(), delayed.size());
    m_stats.numSynchronized++;
    m_stats.avgCoherence = (m_stats.avgCoherence * (m_stats.numSynchronized - 1) +
                            result.coherence) / m_stats.numSynchronized;
    m_stats.avgDelaySamples = (m_stats.avgDelaySamples * (m_stats.numSynchronized - 1) +
                               qAbs(result.delaySamples)) / m_stats.numSynchronized;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit delayEstimated(result.delaySamples, result.coherence, timer.elapsed());
    return result;
}

/* ---- Apply fractional delay ---- */

QVector<double> SignalSynchronizer6::applyDelay(const QVector<double>& signal, double delaySamples)
{
    int n = signal.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double srcIdx = i - delaySamples;
        if (srcIdx < 0 || srcIdx >= n - 1) {
            output[i] = 0.0;
        } else {
            output[i] = sincInterpolate(signal, srcIdx);
        }
    }
    return output;
}

/* ---- Full synchronize ---- */

QVector<double> SignalSynchronizer6::synchronize(const QVector<double>& reference,
                                                  const QVector<double>& delayed)
{
    QElapsedTimer timer;
    timer.start();

    SyncResult result = estimateDelay(reference, delayed);
    QVector<double> aligned = applyDelay(delayed, result.delaySamples);

    emit syncCompleted(result.delaySamples, timer.elapsed());
    return aligned;
}

/* ---- Reset ---- */

void SignalSynchronizer6::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
