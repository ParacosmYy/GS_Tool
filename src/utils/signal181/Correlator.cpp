/**
 * @file Correlator.cpp
 * @brief Correlator 实现
 *
 * 实现互相关/自相关：FFT加速、时延估计、归一化系数、互功率谱。
 */

#include "utils/signal181/Correlator.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Correlator::Correlator(QObject *parent) : QObject(parent) {}
Correlator::~Correlator() = default;

/* ---- Configuration ---- */

void Correlator::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void Correlator::setNormalized(bool enabled) { m_normalized = enabled; }

/* ---- Next power of 2 ---- */

int Correlator::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Radix-2 Cooley-Tukey FFT ---- */

void Correlator::fft(QVector<double>& re, QVector<double>& im, bool inverse) const
{
    int N = re.size();
    if (N <= 1) return;

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    // Butterfly stages
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= N; len <<= 1) {
        double angle = sign * 2.0 * M_PI / len;
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        for (int i = 0; i < N; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < N; ++i) {
            re[i] /= N;
            im[i] /= N;
        }
    }
}

/* ---- Direct correlation for small signals ---- */

QVector<double> Correlator::directCorrelate(const QVector<double>& x,
                                              const QVector<double>& y) const
{
    int nx = x.size();
    int ny = y.size();
    int len = nx + ny - 1;
    QVector<double> result(len, 0.0);

    for (int lag = -(ny - 1); lag < nx; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < ny; ++i) {
            int idx = lag + i;
            if (idx >= 0 && idx < nx)
                sum += x[idx] * y[i];
        }
        result[lag + ny - 1] = sum;
    }
    return result;
}

/* ---- FFT-accelerated cross-correlation ---- */

QVector<double> Correlator::crossCorrelate(const QVector<double>& x,
                                             const QVector<double>& y)
{
    QElapsedTimer timer;
    timer.start();

    int nx = x.size();
    int ny = y.size();
    if (nx == 0 || ny == 0) return {};

    // For small signals, use direct method
    if (nx < 64 && ny < 64) {
        auto result = directCorrelate(x, y);
        m_stats.totalCorrelations++;
        m_stats.signalLength = nx;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCorrelations;
        return result;
    }

    // FFT method: pad to next power of 2
    int N = nextPow2(nx + ny - 1);
    QVector<double> xRe(N, 0.0), xIm(N, 0.0);
    QVector<double> yRe(N, 0.0), yIm(N, 0.0);

    for (int i = 0; i < nx; ++i) xRe[i] = x[i];
    for (int i = 0; i < ny; ++i) yRe[i] = y[i];

    // Forward FFT
    fft(xRe, xIm, false);
    fft(yRe, yIm, false);

    // Cross-power spectrum: conj(X) * Y
    for (int i = 0; i < N; ++i) {
        double r = xRe[i] * yRe[i] + xIm[i] * yIm[i];
        double im = xRe[i] * yIm[i] - xIm[i] * yRe[i];
        xRe[i] = r;
        xIm[i] = im;
    }

    // Inverse FFT
    fft(xRe, xIm, true);

    // Extract correlation result (centered)
    int resultLen = nx + ny - 1;
    QVector<double> result(resultLen);
    int offset = ny - 1;
    for (int i = 0; i < resultLen; ++i) {
        int idx = (i - offset + N) % N;
        result[i] = xRe[idx];
    }

    // Normalize if requested
    if (m_normalized) {
        double normX = 0.0, normY = 0.0;
        for (double v : x) normX += v * v;
        for (double v : y) normY += v * v;
        double norm = qSqrt(normX * normY + 1e-15);
        for (auto& v : result) v /= norm;
    }

    // Find peak
    double peak = 0.0;
    int peakIdx = 0;
    for (int i = 0; i < result.size(); ++i)
        if (qAbs(result[i]) > peak) {
            peak = qAbs(result[i]);
            peakIdx = i;
        }

    m_stats.totalCorrelations++;
    m_stats.signalLength = nx;
    m_stats.peakLag = peakIdx - (ny - 1);
    m_stats.peakCorrelation = peak;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCorrelations;

    emit correlationCompleted(resultLen, m_stats.peakLag);
    return result;
}

/* ---- Auto-correlation ---- */

QVector<double> Correlator::autoCorrelate(const QVector<double>& x)
{
    return crossCorrelate(x, x);
}

/* ---- Estimate lag in samples ---- */

int Correlator::estimateLag(const QVector<double>& xcorr) const
{
    if (xcorr.isEmpty()) return 0;

    double peak = 0.0;
    int peakIdx = 0;
    for (int i = 0; i < xcorr.size(); ++i)
        if (qAbs(xcorr[i]) > peak) {
            peak = qAbs(xcorr[i]);
            peakIdx = i;
        }

    int center = xcorr.size() / 2;
    return peakIdx - center;
}

/* ---- Estimate lag in milliseconds ---- */

double Correlator::estimateLagMs(const QVector<double>& xcorr) const
{
    int lag = estimateLag(xcorr);
    return lag * 1000.0 / m_sampleRate;
}

/* ---- Normalized correlation coefficient (Pearson) ---- */

double Correlator::normalizedCoefficient(const QVector<double>& x,
                                           const QVector<double>& y) const
{
    int n = qMin(x.size(), y.size());
    if (n == 0) return 0.0;

    double meanX = 0.0, meanY = 0.0;
    for (int i = 0; i < n; ++i) { meanX += x[i]; meanY += y[i]; }
    meanX /= n;
    meanY /= n;

    double num = 0.0, denX = 0.0, denY = 0.0;
    for (int i = 0; i < n; ++i) {
        double dx = x[i] - meanX;
        double dy = y[i] - meanY;
        num += dx * dy;
        denX += dx * dx;
        denY += dy * dy;
    }
    return num / (qSqrt(denX * denY) + 1e-15);
}

/* ---- Cross power spectrum ---- */

QPair<QVector<double>, QVector<double>> Correlator::crossPowerSpectrum(
    const QVector<double>& x, const QVector<double>& y)
{
    int N = nextPow2(qMax(x.size(), y.size()));
    QVector<double> xRe(N, 0.0), xIm(N, 0.0);
    QVector<double> yRe(N, 0.0), yIm(N, 0.0);

    for (int i = 0; i < x.size(); ++i) xRe[i] = x[i];
    for (int i = 0; i < y.size(); ++i) yRe[i] = y[i];

    fft(xRe, xIm, false);
    fft(yRe, yIm, false);

    QVector<double> mag(N), phase(N);
    for (int i = 0; i < N; ++i) {
        double r = xRe[i] * yRe[i] + xIm[i] * yIm[i];
        double im = xRe[i] * yIm[i] - xIm[i] * yRe[i];
        mag[i] = qSqrt(r * r + im * im);
        phase[i] = qAtan2(im, r);
    }
    return {mag, phase};
}

/* ---- Reset ---- */

void Correlator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
