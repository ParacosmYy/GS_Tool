/**
 * @file ZoomFFT3.cpp
 * @brief ZoomFFT3 实现
 *
 * 实现缩放FFT：复数频移、多相抽取、FFT变换。
 */

#include "utils/fft215/ZoomFFT3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ZoomFFT3::ZoomFFT3(QObject *parent) : QObject(parent)
{
    designFilter();
}

ZoomFFT3::~ZoomFFT3() = default;

/* ---- Design polyphase lowpass filter ---- */

void ZoomFFT3::designFilter()
{
    // Lowpass filter for decimation: cutoff = sampleRate / (2 * decimation)
    int order = m_filterOrder;
    m_filterCoeffs.resize(order);
    double cutoff = 1.0 / (2.0 * m_decimation);
    double sum = 0.0;

    for (int i = 0; i < order; ++i) {
        double n = i - (order - 1) / 2.0;
        double x = 2.0 * M_PI * cutoff * n;
        double sinc = (qAbs(n) < 1e-10) ? 1.0 : qSin(x) / x;
        // Blackman window
        double w = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (order - 1))
                 + 0.08 * qCos(4.0 * M_PI * i / (order - 1));
        m_filterCoeffs[i] = sinc * w;
        sum += m_filterCoeffs[i];
    }
    // Normalize
    for (auto& c : m_filterCoeffs) c /= sum;
}

/* ---- Configuration ---- */

void ZoomFFT3::setParameters(int fftSize, double centerFreq,
                              double bandwidth, int decimationFactor,
                              double sampleRate)
{
    m_fftSize = qMax(16, fftSize);
    m_centerFreq = centerFreq;
    m_bandwidth = qMax(1.0, bandwidth);
    m_decimation = qMax(1, decimationFactor);
    m_sampleRate = qMax(8000.0, sampleRate);
    m_filterOrder = 8 * m_decimation;

    m_stats.fftSize = m_fftSize;
    m_stats.centerFreq = m_centerFreq;
    m_stats.bandwidth = m_bandwidth;
    m_stats.decimationFactor = m_decimation;

    designFilter();
}

/* ---- Twiddle factor ---- */

void ZoomFFT3::twiddle(int k, int N, double& wr, double& wi)
{
    double angle = -2.0 * M_PI * k / N;
    wr = qCos(angle);
    wi = qSin(angle);
}

/* ---- Bit-reversal permutation ---- */

void ZoomFFT3::bitReverse(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    int bits = 0;
    while ((1 << bits) < n) ++bits;

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        int val = i;
        for (int b = 0; b < bits; ++b) {
            rev = (rev << 1) | (val & 1);
            val >>= 1;
        }
        if (rev > i) {
            std::swap(real[i], real[rev]);
            std::swap(imag[i], imag[rev]);
        }
    }
}

/* ---- Complex frequency shift (heterodyne) ---- */

void ZoomFFT3::frequencyShift(QVector<double>& real, QVector<double>& imag,
                               double shiftFreq) const
{
    int n = real.size();
    for (int i = 0; i < n; ++i) {
        double angle = -2.0 * M_PI * shiftFreq * i / m_sampleRate;
        double wr = qCos(angle);
        double wi = qSin(angle);
        double r = real[i] * wr - imag[i] * wi;
        double im = real[i] * wi + imag[i] * wr;
        real[i] = r;
        imag[i] = im;
    }
}

/* ---- Polyphase decimation ---- */

void ZoomFFT3::decimate(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    int outLen = n / m_decimation;
    if (outLen < 1) return;

    QVector<double> rOut(outLen, 0.0);
    QVector<double> iOut(outLen, 0.0);

    for (int i = 0; i < outLen; ++i) {
        double rSum = 0.0, iSum = 0.0;
        for (int j = 0; j < m_filterCoeffs.size(); ++j) {
            int idx = i * m_decimation + j - m_filterCoeffs.size() / 2;
            if (idx >= 0 && idx < n) {
                rSum += real[idx] * m_filterCoeffs[j];
                iSum += imag[idx] * m_filterCoeffs[j];
            }
        }
        rOut[i] = rSum;
        iOut[i] = iSum;
    }

    real = rOut;
    imag = iOut;
}

/* ---- FFT on complex data (Cooley-Tukey radix-2) ---- */

QVector<double> ZoomFFT3::fftComplex(const QVector<double>& real,
                                      const QVector<double>& imag) const
{
    int n = real.size();
    if (n <= 1) {
        QVector<double> mag(1);
        mag[0] = qSqrt(real[0] * real[0] + imag[0] * imag[0]);
        return mag;
    }

    QVector<double> re = real;
    QVector<double> im = imag;

    bitReverse(re, im);

    for (int len = 2; len <= n; len *= 2) {
        int half = len / 2;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; ++j) {
                double wr, wi;
                twiddle(j, len, wr, wi);
                double tr = re[i + j + half] * wr - im[i + j + half] * wi;
                double ti = re[i + j + half] * wi + im[i + j + half] * wr;
                re[i + j + half] = re[i + j] - tr;
                im[i + j + half] = im[i + j] - ti;
                re[i + j] += tr;
                im[i + j] += ti;
            }
        }
    }

    // Magnitude
    QVector<double> mag(n);
    for (int i = 0; i < n; ++i)
        mag[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);

    return mag;
}

/* ---- Process real input ---- */

QVector<double> ZoomFFT3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    // Create analytic signal (Hilbert transform approximation)
    int n = input.size();
    QVector<double> re = input;
    QVector<double> im(n, 0.0);

    // Simple Hilbert via FFT: zero negative frequencies
    if (n > 1) {
        QVector<double> rCopy = re;
        QVector<double> iCopy = im;
        bitReverse(rCopy, iCopy);
        for (int len = 2; len <= n; len *= 2) {
            int half = len / 2;
            for (int i = 0; i < n; i += len) {
                for (int j = 0; j < half; ++j) {
                    double wr, wi;
                    twiddle(j, len, wr, wi);
                    double tr = rCopy[i + j + half] * wr - iCopy[i + j + half] * wi;
                    double ti = rCopy[i + j + half] * wi + iCopy[i + j + half] * wr;
                    rCopy[i + j + half] = rCopy[i + j] - tr;
                    iCopy[i + j + half] = iCopy[i + j] - ti;
                    rCopy[i + j] += tr;
                    iCopy[i + j] += ti;
                }
            }
        }
        for (int i = 1; i < n / 2; ++i) { rCopy[i] *= 2; iCopy[i] *= 2; }
        for (int i = n / 2 + 1; i < n; ++i) { rCopy[i] = 0; iCopy[i] = 0; }
        // Inverse FFT
        for (auto& v : iCopy) v = -v;
        bitReverse(rCopy, iCopy);
        for (int len = 2; len <= n; len *= 2) {
            int half = len / 2;
            for (int i = 0; i < n; i += len) {
                for (int j = 0; j < half; ++j) {
                    double wr, wi;
                    twiddle(j, len, wr, wi);
                    double tr = rCopy[i+j+half]*wr - iCopy[i+j+half]*wi;
                    double ti = rCopy[i+j+half]*wi + iCopy[i+j+half]*wr;
                    rCopy[i+j+half] = rCopy[i+j] - tr;
                    iCopy[i+j+half] = iCopy[i+j] - ti;
                    rCopy[i+j] += tr;
                    iCopy[i+j] += ti;
                }
            }
        }
        re = rCopy;
        im = iCopy;
        for (auto& v : im) v = -v;
        for (auto& v : re) v /= n;
        for (auto& v : im) v /= n;
    }

    auto result = processComplex(re, im);
    m_stats.inputSize = n;
    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit transformCompleted(result.size(), m_centerFreq, timer.elapsed());
    return result;
}

/* ---- Process complex I/Q data ---- */

QVector<double> ZoomFFT3::processComplex(const QVector<double>& real,
                                          const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re = real;
    QVector<double> im = imag;

    // Step 1: Frequency shift to center the zoom band
    frequencyShift(re, im, m_centerFreq);

    // Step 2: Lowpass filter and decimate
    decimate(re, im);

    // Step 3: Zero-pad to FFT size
    re.resize(m_fftSize, 0.0);
    im.resize(m_fftSize, 0.0);

    // Step 4: FFT
    QVector<double> spectrum = fftComplex(re, im);

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit transformCompleted(spectrum.size(), m_centerFreq, timer.elapsed());

    return spectrum;
}

/* ---- Frequency axis ---- */

QVector<double> ZoomFFT3::frequencyAxis() const
{
    double binWidth = m_bandwidth / m_fftSize;
    double startFreq = m_centerFreq - m_bandwidth / 2.0;
    QVector<double> freqs(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i)
        freqs[i] = startFreq + i * binWidth;
    return freqs;
}

/* ---- Reset ---- */

void ZoomFFT3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_filterCoeffs.clear();
    designFilter();
}
