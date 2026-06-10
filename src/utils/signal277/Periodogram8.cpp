/**
 * @file Periodogram8.cpp
 * @brief Periodogram8 实现
 *
 * 实现周期图：Bartlett平均周期图与Welch重叠段方差降低谱估计。
 */

#include "utils/signal277/Periodogram8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Periodogram8::Periodogram8(QObject *parent)
    : QObject(parent) {}

Periodogram8::~Periodogram8() = default;

/* ---- Configuration ---- */

void Periodogram8::setSegmentLength(int len) { m_segmentLen = qBound(16, len, 1 << 20); }
void Periodogram8::setOverlapRatio(double ratio) { m_overlapRatio = qBound(0.0, ratio, 0.9); }
void Periodogram8::setWindow(Window win) { m_window = win; }
void Periodogram8::setFFTSize(int n) { m_fftSize = (n == 0) ? 0 : qBound(16, n, 1 << 20); }

/* ---- Generate window coefficients ---- */

QVector<double> Periodogram8::generateWindow(int len) const
{
    QVector<double> w(len);
    for (int n = 0; n < len; ++n) {
        switch (m_window) {
        case Rectangular:
            w[n] = 1.0; break;
        case Hanning:
            w[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (len - 1))); break;
        case Hamming:
            w[n] = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (len - 1)); break;
        case Blackman:
            w[n] = 0.42 - 0.5 * qCos(2.0 * M_PI * n / (len - 1))
                   + 0.08 * qCos(4.0 * M_PI * n / (len - 1)); break;
        case Bartlett:
            w[n] = 1.0 - qAbs(2.0 * n / (len - 1) - 1.0); break;
        }
    }
    return w;
}

/* ---- Next power of 2 ---- */

int Periodogram8::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Bit-reversal permutation ---- */

void Periodogram8::bitReverse(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    int bits = 0;
    { int t = n; while (t > 1) { bits++; t >>= 1; } }
    for (int i = 0; i < n; ++i) {
        int rev = 0, val = i;
        for (int b = 0; b < bits; ++b) { rev = (rev << 1) | (val & 1); val >>= 1; }
        if (i < rev) {
            std::swap(re[i], re[rev]);
            std::swap(im[i], im[rev]);
        }
    }
}

/* ---- Radix-2 FFT (in-place) ---- */

void Periodogram8::fftReal(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    bitReverse(re, im);

    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        for (int i = 0; i < n; i += len) {
            for (int k = 0; k < len / 2; ++k) {
                double wr = qCos(angle * k), wi = qSin(angle * k);
                double tRe = re[i + k + len/2] * wr - im[i + k + len/2] * wi;
                double tIm = re[i + k + len/2] * wi + im[i + k + len/2] * wr;
                re[i + k + len/2] = re[i + k] - tRe;
                im[i + k + len/2] = im[i + k] - tIm;
                re[i + k] += tRe;
                im[i + k] += tIm;
            }
        }
    }
}

/* ---- Raw periodogram ---- */

QVector<double> Periodogram8::rawPeriodogram(const QVector<double>& signal, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    int fftN = (m_fftSize > 0) ? m_fftSize : nextPow2(n);
    int halfN = fftN / 2;

    QVector<double> re(fftN, 0.0), im(fftN, 0.0);
    QVector<double> win = generateWindow(n);
    double winPower = 0.0;
    for (int i = 0; i < n; ++i) {
        re[i] = signal[i] * win[i];
        winPower += win[i] * win[i];
    }

    fftReal(re, im);

    // Compute power spectral density
    double scale = sampleRate * winPower;
    QVector<double> psd(halfN + 1);
    for (int k = 0; k <= halfN; ++k)
        psd[k] = (re[k] * re[k] + im[k] * im[k]) / scale;

    // Frequency bins
    m_freqBins.resize(halfN + 1);
    for (int k = 0; k <= halfN; ++k)
        m_freqBins[k] = k * sampleRate / fftN;

    double elapsed = timer.elapsed();
    m_stats.signalLength = n;
    m_stats.numSegments = 1;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit estimationDone(1, fftN, elapsed);

    return psd;
}

/* ---- Bartlett's method: non-overlapping averaged periodogram ---- */

QVector<double> Periodogram8::bartlett(const QVector<double>& signal, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    int segLen = qMin(m_segmentLen, n);
    int fftN = (m_fftSize > 0) ? m_fftSize : nextPow2(segLen);
    int halfN = fftN / 2;
    int numSegs = n / segLen;

    if (numSegs < 1) return rawPeriodogram(signal, sampleRate);

    QVector<double> avgPsd(halfN + 1, 0.0);
    QVector<double> win = generateWindow(segLen);
    double winPower = 0.0;
    for (int i = 0; i < segLen; ++i) winPower += win[i] * win[i];

    for (int s = 0; s < numSegs; ++s) {
        int offset = s * segLen;
        QVector<double> re(fftN, 0.0), im(fftN, 0.0);
        for (int i = 0; i < segLen; ++i)
            re[i] = signal[offset + i] * win[i];

        fftReal(re, im);

        double scale = sampleRate * winPower;
        for (int k = 0; k <= halfN; ++k)
            avgPsd[k] += (re[k] * re[k] + im[k] * im[k]) / scale;
    }

    for (int k = 0; k <= halfN; ++k) avgPsd[k] /= numSegs;

    m_freqBins.resize(halfN + 1);
    for (int k = 0; k <= halfN; ++k)
        m_freqBins[k] = k * sampleRate / fftN;

    double elapsed = timer.elapsed();
    m_stats.signalLength = n;
    m_stats.segmentLength = segLen;
    m_stats.numSegments = numSegs;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit estimationDone(numSegs, fftN, elapsed);

    return avgPsd;
}

/* ---- Welch's method: overlapping averaged periodogram ---- */

QVector<double> Periodogram8::welch(const QVector<double>& signal, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    int segLen = qMin(m_segmentLen, n);
    int fftN = (m_fftSize > 0) ? m_fftSize : nextPow2(segLen);
    int halfN = fftN / 2;
    int hopSize = static_cast<int>(segLen * (1.0 - m_overlapRatio));
    if (hopSize < 1) hopSize = 1;

    int numSegs = (n - segLen) / hopSize + 1;
    if (numSegs < 1) numSegs = 1;

    QVector<double> avgPsd(halfN + 1, 0.0);
    QVector<double> win = generateWindow(segLen);
    double winPower = 0.0;
    for (int i = 0; i < segLen; ++i) winPower += win[i] * win[i];

    // Overlap-add normalization
    QVector<double> norm(halfN + 1, 0.0);

    for (int s = 0; s < numSegs; ++s) {
        int offset = s * hopSize;
        QVector<double> re(fftN, 0.0), im(fftN, 0.0);
        for (int i = 0; i < segLen && offset + i < n; ++i)
            re[i] = signal[offset + i] * win[i];

        fftReal(re, im);

        double scale = sampleRate * winPower;
        for (int k = 0; k <= halfN; ++k)
            avgPsd[k] += (re[k] * re[k] + im[k] * im[k]) / scale;
    }

    for (int k = 0; k <= halfN; ++k) avgPsd[k] /= numSegs;

    m_freqBins.resize(halfN + 1);
    for (int k = 0; k <= halfN; ++k)
        m_freqBins[k] = k * sampleRate / fftN;

    double elapsed = timer.elapsed();
    m_stats.signalLength = n;
    m_stats.segmentLength = segLen;
    m_stats.numSegments = numSegs;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit estimationDone(numSegs, fftN, elapsed);

    return avgPsd;
}

/* ---- Accessors ---- */

QVector<double> Periodogram8::frequencyBins() const { return m_freqBins; }

QVector<double> Periodogram8::psdDb(const QVector<double>& psd) const
{
    QVector<double> db(psd.size());
    for (int i = 0; i < psd.size(); ++i)
        db[i] = (psd[i] > 1e-20) ? 10.0 * qLn(psd[i]) / M_LN10 : -200.0;
    return db;
}

/* ---- Reset ---- */

void Periodogram8::resetStatistics()
{
    m_freqBins.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
