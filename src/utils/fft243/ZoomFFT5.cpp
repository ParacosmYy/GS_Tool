/**
 * @file ZoomFFT5.cpp
 * @brief ZoomFFT5 实现
 *
 * 实现缩放FFT：带通抽取与复数解调窄带高分辨率频谱分析。
 */

#include "utils/fft243/ZoomFFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ZoomFFT5::ZoomFFT5(QObject *parent) : QObject(parent) {}
ZoomFFT5::~ZoomFFT5() = default;

/* ---- Configuration ---- */

void ZoomFFT5::setCenterFrequency(double freq) { m_centerFreq = freq; }
void ZoomFFT5::setDecimationFactor(int d) { m_decimation = qMax(1, d); }
void ZoomFFT5::setFFTSize(int n) { m_fftSize = qMax(4, n); }
void ZoomFFT5::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void ZoomFFT5::setWindowType(int type) { m_windowType = qBound(0, type, 2); }

/* ---- Complex demodulation ---- */

void ZoomFFT5::demodulate(const QVector<double>& in,
                           QVector<double>& outRe, QVector<double>& outIm) const
{
    int n = in.size();
    outRe.resize(n);
    outIm.resize(n);
    double omega = 2.0 * M_PI * m_centerFreq / m_sampleRate;

    for (int i = 0; i < n; ++i) {
        double phase = -omega * i;
        outRe[i] = in[i] * qCos(phase);
        outIm[i] = in[i] * qSin(phase);
    }
}

/* ---- Design simple FIR lowpass ---- */

QVector<double> ZoomFFT5::designLowpass(int len, double cutoff) const
{
    QVector<double> h(len, 0.0);
    int mid = len / 2;
    double omega = 2.0 * M_PI * cutoff / m_sampleRate;
    for (int i = 0; i < len; ++i) {
        int n = i - mid;
        if (n == 0) {
            h[i] = omega / M_PI;
        } else {
            h[i] = qSin(omega * n) / (M_PI * n);
        }
        // Hamming window
        double w = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (len - 1));
        h[i] *= w;
    }
    return h;
}

/* ---- Decimate ---- */

void ZoomFFT5::decimate(const QVector<double>& re, const QVector<double>& im,
                         QVector<double>& outRe, QVector<double>& outIm) const
{
    int n = re.size();
    int filtLen = qMin(31, n / 2);
    if (filtLen < 3) {
        outRe = re; outIm = im;
        return;
    }

    // Anti-aliasing lowpass filter
    double cutoff = m_sampleRate / (2.0 * m_decimation);
    QVector<double> h = designLowpass(filtLen, cutoff);

    // Filter + downsample
    int outSize = n / m_decimation;
    outRe.resize(outSize);
    outIm.resize(outSize);

    for (int i = 0; i < outSize; ++i) {
        int si = i * m_decimation;
        double sr = 0.0, si2 = 0.0;
        for (int j = 0; j < filtLen; ++j) {
            int idx = si - j;
            if (idx >= 0 && idx < n) {
                sr += re[idx] * h[j];
                si2 += im[idx] * h[j];
            }
        }
        outRe[i] = sr;
        outIm[i] = si2;
    }
}

/* ---- Apply window ---- */

void ZoomFFT5::applyWindow(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    for (int i = 0; i < n; ++i) {
        double w = 1.0;
        if (m_windowType == 1)      // Hann
            w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        else if (m_windowType == 2) // Hamming
            w = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (n - 1));
        re[i] *= w;
        im[i] *= w;
    }
}

/* ---- Radix-2 complex FFT ---- */

void ZoomFFT5::fft(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
    // FFT butterfly
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wRe = qCos(ang), wIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
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
}

/* ---- Process ---- */

QVector<double> ZoomFFT5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Complex demodulation (frequency shift to baseband)
    QVector<double> demodRe, demodIm;
    demodulate(input, demodRe, demodIm);

    // Step 2: Low-pass filter + decimation
    QVector<double> decRe, decIm;
    decimate(demodRe, demodIm, decRe, decIm);

    // Step 3: Pad/truncate to FFT size
    int n = m_fftSize;
    QVector<double> fftRe(n, 0.0), fftIm(n, 0.0);
    int copyLen = qMin(decRe.size(), n);
    for (int i = 0; i < copyLen; ++i) {
        fftRe[i] = decRe[i];
        fftIm[i] = decIm[i];
    }

    // Step 4: Apply window
    applyWindow(fftRe, fftIm);

    // Step 5: FFT
    fft(fftRe, fftIm);

    // Step 6: Compute magnitude
    QVector<double> mag(n);
    for (int i = 0; i < n; ++i)
        mag[i] = qSqrt(fftRe[i] * fftRe[i] + fftIm[i] * fftIm[i]) / n;

    m_stats.inputSize = input.size();
    m_stats.fftSize = n;
    m_stats.decimationFactor = m_decimation;
    m_stats.centerFreqHz = m_centerFreq;
    m_stats.bandwidthHz = m_sampleRate / m_decimation;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit zoomCompleted(input.size(), n, timer.elapsed());
    return mag;
}

/* ---- Frequency axis ---- */

QVector<double> ZoomFFT5::frequencyAxis() const
{
    double bw = m_sampleRate / m_decimation;
    double startFreq = m_centerFreq - bw / 2.0;
    QVector<double> freq(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i)
        freq[i] = startFreq + i * bw / m_fftSize;
    return freq;
}

/* ---- Reset ---- */

void ZoomFFT5::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
