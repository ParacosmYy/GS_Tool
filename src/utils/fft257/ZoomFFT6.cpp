/**
 * @file ZoomFFT6.cpp
 * @brief ZoomFFT6 实现
 *
 * 实现缩放FFT：复数频移抽取与多相分析滤波窄带高分辨率分析。
 */

#include "utils/fft257/ZoomFFT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ZoomFFT6::ZoomFFT6(QObject *parent)
    : QObject(parent)
{
    designPolyphaseFilter();
}
ZoomFFT6::~ZoomFFT6() = default;

/* ---- Configuration ---- */

void ZoomFFT6::setCenterFrequency(double fc) { m_centerFreq = qBound(0.0, fc, 0.5); }
void ZoomFFT6::setBandwidth(double bw) { m_bandwidth = qBound(0.01, bw, 0.5); }
void ZoomFFT6::setFFTSize(int size)
{
    // Power of 2
    int p = 1;
    while (p < size) p <<= 1;
    m_fftSize = p;
}
void ZoomFFT6::setDecimationFactor(int d) { m_decimFactor = qMax(1, d); }
void ZoomFFT6::setPolyphaseBranches(int branches) { m_polyBranches = qMax(2, branches); }

/* ---- Design polyphase lowpass filter ---- */

void ZoomFFT6::designPolyphaseFilter()
{
    // Design a lowpass filter for the zoom bandwidth
    int filterLen = m_polyBranches * m_decimFactor * 4; // 4 taps per branch per phase
    double cutoff = m_bandwidth / 2.0;

    m_polyCoeffs.resize(filterLen);
    double sum = 0.0;
    for (int i = 0; i < filterLen; ++i) {
        double n = i - filterLen / 2.0;
        double sinc;
        if (qFuzzyIsNull(n))
            sinc = 1.0;
        else
            sinc = qSin(M_PI * cutoff * n) / (M_PI * n);

        // Hamming window
        double window = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (filterLen - 1));
        m_polyCoeffs[i] = sinc * window;
        sum += m_polyCoeffs[i];
    }
    // Normalize
    for (int i = 0; i < filterLen; ++i)
        m_polyCoeffs[i] /= sum;

    // Initialize branch delay lines
    m_polyState.resize(m_polyBranches);
    int tapsPerBranch = filterLen / m_polyBranches;
    for (int b = 0; b < m_polyBranches; ++b)
        m_polyState[b].resize(tapsPerBranch, 0.0);
}

/* ---- Complex frequency shift (heterodyne) ---- */

void ZoomFFT6::frequencyShift(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    for (int i = 0; i < n; ++i) {
        double phase = 2.0 * M_PI * m_centerFreq * i;
        double cosP = qCos(phase);
        double sinP = qSin(phase);
        double r = real[i] * cosP + imag[i] * sinP;
        double im = -real[i] * sinP + imag[i] * cosP;
        real[i] = r;
        imag[i] = im;
    }
}

/* ---- Polyphase decimation filter ---- */

QVector<double> ZoomFFT6::polyphaseDecimate(const QVector<double>& real,
                                             const QVector<double>& imag) const
{
    int n = real.size();
    int d = m_decimFactor;
    int outLen = n / d;
    QVector<double> filtered;
    filtered.reserve(outLen);

    for (int i = 0; i < outLen; ++i) {
        int base = i * d;
        double mag = 0.0;
        for (int b = 0; b < m_polyBranches; ++b) {
            int srcIdx = base + b;
            if (srcIdx < n) {
                double coeff = (b < m_polyCoeffs.size()) ? m_polyCoeffs[b] : 0.0;
                double r = real[srcIdx] * coeff;
                double im = imag[srcIdx] * coeff;
                mag += r * r + im * im;
            }
        }
        filtered.append(qSqrt(mag));
    }
    return filtered;
}

/* ---- Bit-reversal ---- */

int ZoomFFT6::bitReverse(int x, int bits)
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/* ---- Twiddle factor ---- */

void ZoomFFT6::twiddle(int k, int n, double& wr, double& wi)
{
    double angle = -2.0 * M_PI * k / n;
    wr = qCos(angle);
    wi = qSin(angle);
}

/* ---- Compute FFT magnitude ---- */

QVector<double> ZoomFFT6::computeFFT(const QVector<double>& samples) const
{
    int n = samples.size();
    int bits = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; bits++; }

    // Bit-reverse copy (using real/imag arrays)
    QVector<double> re(n, 0.0);
    QVector<double> im(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int j = bitReverse(i, bits);
        re[j] = samples[i];
    }

    // Cooley-Tukey radix-2 in-place FFT
    for (int len = 2; len <= n; len <<= 1) {
        int halfLen = len / 2;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < halfLen; ++j) {
                double wr, wi;
                twiddle(j, len, wr, wi);
                double tr = wr * re[i + j + halfLen] - wi * im[i + j + halfLen];
                double ti = wr * im[i + j + halfLen] + wi * re[i + j + halfLen];
                re[i + j + halfLen] = re[i + j] - tr;
                im[i + j + halfLen] = im[i + j] - ti;
                re[i + j] += tr;
                im[i + j] += ti;
            }
        }
    }

    // Magnitude
    QVector<double> mag(n / 2);
    for (int i = 0; i < n / 2; ++i)
        mag[i] = qSqrt(re[i] * re[i] + im[i] * im[i]) / n;

    return mag;
}

/* ---- Main process ---- */

QVector<double> ZoomFFT6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    designPolyphaseFilter();

    int n = input.size();
    if (n < m_decimFactor * 2) return {};

    // Step 1: Complex frequency shift to center the zoom band at DC
    QVector<double> real(n);
    QVector<double> imag(n, 0.0);
    for (int i = 0; i < n; ++i)
        real[i] = input[i];

    frequencyShift(real, imag);

    // Step 2: Polyphase decimation
    QVector<double> decimated = polyphaseDecimate(real, imag);
    int decLen = decimated.size();

    // Step 3: Pad to FFT size
    if (decLen < m_fftSize) {
        decimated.resize(m_fftSize, 0.0);
    } else if (decLen > m_fftSize) {
        decimated.resize(m_fftSize);
    }

    // Step 4: Compute FFT
    QVector<double> spectrum = computeFFT(decimated);

    // Keep only the zoom bandwidth portion
    int zoomBins = qMax(1, m_fftSize * m_bandwidth / 2);
    QVector<double> zoomed;
    zoomed.reserve(zoomBins);
    for (int i = 0; i < zoomBins && i < spectrum.size(); ++i)
        zoomed.append(spectrum[i]);

    double elapsed = timer.elapsed();
    m_stats.fftSize = m_fftSize;
    m_stats.decimationFactor = m_decimFactor;
    m_stats.zoomBands = zoomed.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit analysisCompleted(zoomed.size(), m_centerFreq, elapsed);
    return zoomed;
}

/* ---- Frequency axis ---- */

QVector<double> ZoomFFT6::frequencyAxis() const
{
    int zoomBins = qMax(1, m_fftSize * m_bandwidth / 2);
    QVector<double> freq(zoomBins);
    double binWidth = m_bandwidth / m_fftSize;
    double start = m_centerFreq - m_bandwidth / 4.0;
    for (int i = 0; i < zoomBins; ++i)
        freq[i] = start + i * binWidth;
    return freq;
}

/* ---- Reset ---- */

void ZoomFFT6::resetStatistics()
{
    m_polyCoeffs.clear();
    m_polyState.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
