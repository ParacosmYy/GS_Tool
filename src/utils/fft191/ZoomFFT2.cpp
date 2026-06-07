/**
 * @file ZoomFFT2.cpp
 * @brief ZoomFFT2 实现
 *
 * 实现Zoom FFT：频移、低通滤波、降采样抽取、高分辨率频谱细化。
 */

#include "utils/fft191/ZoomFFT2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ZoomFFT2::ZoomFFT2(QObject *parent) : QObject(parent) {}
ZoomFFT2::~ZoomFFT2() = default;

/* ---- Configuration ---- */

void ZoomFFT2::setInputSize(int N) { m_inputSize = qMax(16, N); }
void ZoomFFT2::setSampleRate(double sr) { m_sampleRate = qMax(1.0, sr); }
void ZoomFFT2::setCenterFrequency(double fc) { m_centerFreq = qMax(0.0, fc); }
void ZoomFFT2::setZoomBandwidth(double bw) { m_bandwidth = qMax(1.0, bw); }
void ZoomFFT2::setDecimationFactor(int D) { m_decimation = qMax(1, D); }

/* ---- Frequency shift (heterodyne to baseband) ---- */

QVector<double> ZoomFFT2::frequencyShift(const QVector<double>& data, double fc) const
{
    int N = data.size();
    QVector<double> shifted(N);
    double w0 = 2.0 * M_PI * fc / m_sampleRate;

    for (int n = 0; n < N; ++n) {
        double phase = -w0 * n;
        // Real-valued shift: multiply by cos(-w0*n)
        shifted[n] = data[n] * qCos(phase);
    }
    return shifted;
}

/* ---- Design FIR lowpass via windowed sinc ---- */

QVector<double> ZoomFFT2::designLowpass(int order, double cutoff) const
{
    QVector<double> h(order + 1);
    double wc = 2.0 * M_PI * cutoff / m_sampleRate;
    int mid = order / 2;

    for (int n = 0; n <= order; ++n) {
        if (n == mid) {
            h[n] = wc / M_PI;
        } else {
            double sinc = qSin(wc * (n - mid)) / (M_PI * (n - mid));
            // Hamming window
            double win = 0.54 - 0.46 * qCos(2.0 * M_PI * n / order);
            h[n] = sinc * win;
        }
    }
    return h;
}

/* ---- Apply FIR lowpass filter ---- */

QVector<double> ZoomFFT2::lowpassFilter(const QVector<double>& data,
                                         double cutoff, int order) const
{
    auto h = designLowpass(order, cutoff);
    QVector<double> out(data.size(), 0.0);

    for (int n = 0; n < data.size(); ++n) {
        double sum = 0.0;
        for (int k = 0; k < h.size(); ++k) {
            int idx = n - k;
            if (idx >= 0 && idx < data.size())
                sum += data[idx] * h[k];
        }
        out[n] = sum;
    }
    return out;
}

/* ---- Decimate by factor D ---- */

QVector<double> ZoomFFT2::decimate(const QVector<double>& data, int D) const
{
    QVector<double> out;
    out.reserve(data.size() / D);
    for (int i = 0; i < data.size(); i += D)
        out.append(data[i]);
    return out;
}

/* ---- Cooley-Tukey radix-2 FFT (magnitudes only) ---- */

QVector<double> ZoomFFT2::fft(const QVector<double>& real) const
{
    int N = real.size();
    // Pad to power of 2
    int N2 = 1;
    while (N2 < N) N2 *= 2;

    QVector<double> re(N2, 0.0), im(N2, 0.0);
    for (int i = 0; i < N; ++i) re[i] = real[i];

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < N2; ++i) {
        int bit = N2 >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    // Butterfly stages
    for (int len = 2; len <= N2; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < N2; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe; im[v] = im[u] - tIm;
                re[u] += tRe; im[u] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }

    // Magnitude (first half)
    int halfN = N2 / 2;
    QVector<double> mag(halfN);
    for (int k = 0; k < halfN; ++k)
        mag[k] = qSqrt(re[k] * re[k] + im[k] * im[k]) / N;
    return mag;
}

/* ---- Main transform ---- */

QVector<double> ZoomFFT2::transform(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Frequency shift to baseband
    auto shifted = frequencyShift(data, m_centerFreq);

    // Step 2: Lowpass filter (half bandwidth)
    double cutoff = m_bandwidth / 2.0;
    int filterOrder = qMin(64, data.size() / 4);
    auto filtered = lowpassFilter(shifted, cutoff, filterOrder);

    // Step 3: Decimate
    auto decimated = decimate(filtered, m_decimation);

    // Step 4: FFT on decimated data (zoomed spectrum)
    auto spectrum = fft(decimated);

    m_stats.totalTransforms++;
    m_stats.inputSize = data.size();
    m_stats.outputSize = spectrum.size();
    m_stats.decimationFactor = m_decimation;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(spectrum.size(), timer.elapsed());
    return spectrum;
}

/* ---- Frequency axis ---- */

QVector<double> ZoomFFT2::frequencyAxis() const
{
    int N = m_inputSize / m_decimation;
    double df = m_sampleRate / (N * m_decimation);
    QVector<double> axis(N / 2);
    double fStart = m_centerFreq - m_bandwidth / 2.0;
    for (int k = 0; k < axis.size(); ++k)
        axis[k] = fStart + k * df;
    return axis;
}

/* ---- Resolution ---- */

double ZoomFFT2::resolution() const
{
    return m_sampleRate / (m_inputSize * m_decimation);
}

/* ---- Reset ---- */

void ZoomFFT2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
