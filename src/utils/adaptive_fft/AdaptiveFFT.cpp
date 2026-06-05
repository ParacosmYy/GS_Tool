/**
 * @file AdaptiveFFT.cpp
 * @brief 自适应FFT频谱分析器实现
 */

#include "AdaptiveFFT.h"
#include <QElapsedTimer>
#include <cmath>

AdaptiveFFT::AdaptiveFFT(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> AdaptiveFFT::analyze(const QVector<double>& data,
                                      double sampleRate, WindowType window)
{
    QElapsedTimer timer;
    timer.start();

    int N = nextPowerOf2(data.size());
    QVector<double> windowed(N, 0.0);
    QVector<double> win = generateWindow(N, window);

    /* 应用窗函数 */
    for (int i = 0; i < qMin(data.size(), N); ++i)
        windowed[i] = data[i] * win[i];

    /* 窗函数补偿系数 */
    double winSum = 0.0;
    for (double w : win) winSum += w;
    double scale = (winSum > 0) ? 2.0 / winSum : 1.0;

    /* FFT */
    QVector<double> imag(N, 0.0);
    inPlaceFFT(windowed, imag);

    /* 计算幅值谱 */
    int halfN = N / 2;
    QVector<double> magnitude(halfN);
    for (int i = 0; i < halfN; ++i) {
        double re = windowed[i], im = imag[i];
        magnitude[i] = std::sqrt(re * re + im * im) * scale;
    }

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit analysisCompleted(N, 0);
    return magnitude;
}

QVector<AdaptiveFFT::Peak> AdaptiveFFT::detectPeaks(
    const QVector<double>& magnitude, double sampleRate, double thresholdDb)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Peak> peaks;
    if (magnitude.size() < 3) return peaks;

    int N = (magnitude.size() - 1) * 2;
    double freqRes = sampleRate / N;

    /* 计算最大幅值用于归一化 */
    double maxMag = *std::max_element(magnitude.begin(), magnitude.end());
    if (maxMag < 1e-15) return peaks;

    double threshold = maxMag * std::pow(10.0, thresholdDb / 20.0);

    /* 计算噪声底 */
    double noiseSum = 0.0;
    int noiseCount = 0;
    for (int i = 1; i < magnitude.size() - 1; ++i) {
        if (magnitude[i] < threshold) {
            noiseSum += magnitude[i] * magnitude[i];
            noiseCount++;
        }
    }
    double noiseFloor = (noiseCount > 0) ? std::sqrt(noiseSum / noiseCount) : 1e-15;

    for (int i = 1; i < magnitude.size() - 1; ++i) {
        if (magnitude[i] > threshold &&
            magnitude[i] > magnitude[i - 1] &&
            magnitude[i] > magnitude[i + 1]) {
            Peak p;
            p.frequency = estimateFrequency(magnitude, i, sampleRate, N);
            p.magnitude = magnitude[i];
            p.phase = 0.0;
            p.snr = 20.0 * std::log10(magnitude[i] / noiseFloor);
            peaks.append(p);
        }
    }

    /* 按幅值降序排序 */
    std::sort(peaks.begin(), peaks.end(),
              [](const Peak& a, const Peak& b) { return a.magnitude > b.magnitude; });

    m_stats.totalPeaksDetected += peaks.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalTransforms);

    return peaks;
}

double AdaptiveFFT::estimateFrequency(const QVector<double>& magnitude,
                                       int peakIndex, double sampleRate,
                                       int fftSize)
{
    if (peakIndex <= 0 || peakIndex >= magnitude.size() - 1)
        return static_cast<double>(peakIndex) * sampleRate / fftSize;

    /* 抛物线插值 */
    double alpha = std::log(magnitude[peakIndex - 1] + 1e-30);
    double beta = std::log(magnitude[peakIndex] + 1e-30);
    double gamma = std::log(magnitude[peakIndex + 1] + 1e-30);

    double delta = 0.5 * (alpha - gamma) / (alpha - 2.0 * beta + gamma);
    double refinedIndex = peakIndex + delta;

    return refinedIndex * sampleRate / fftSize;
}

QVector<double> AdaptiveFFT::generateWindow(int size, WindowType type)
{
    QVector<double> w(size);
    int N = size - 1;
    if (N < 1) { w.fill(1.0); return w; }

    switch (type) {
    case Rectangular:
        w.fill(1.0);
        break;
    case Hanning:
        for (int i = 0; i < size; ++i)
            w[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / N));
        break;
    case Hamming:
        for (int i = 0; i < size; ++i)
            w[i] = 0.54 - 0.46 * std::cos(2.0 * M_PI * i / N);
        break;
    case Blackman:
        for (int i = 0; i < size; ++i)
            w[i] = 0.42 - 0.5 * std::cos(2.0 * M_PI * i / N)
                   + 0.08 * std::cos(4.0 * M_PI * i / N);
        break;
    case FlatTop:
        for (int i = 0; i < size; ++i) {
            double x = 2.0 * M_PI * i / N;
            w[i] = 0.21557895 - 0.41663158 * std::cos(x)
                   + 0.27726316 * std::cos(2.0 * x)
                   - 0.08357895 * std::cos(3.0 * x)
                   + 0.00694737 * std::cos(4.0 * x);
        }
        break;
    }
    return w;
}

QPair<QVector<double>, QVector<double>> AdaptiveFFT::powerSpectralDensity(
    const QVector<double>& data, double sampleRate)
{
    int N = nextPowerOf2(data.size());
    QVector<double> re(N, 0.0);
    QVector<double> im(N, 0.0);
    QVector<double> win = generateWindow(N, Hanning);

    double winSqSum = 0.0;
    for (int i = 0; i < N; ++i) {
        re[i] = (i < data.size()) ? data[i] * win[i] : 0.0;
        winSqSum += win[i] * win[i];
    }

    inPlaceFFT(re, im);

    int halfN = N / 2;
    double freqRes = sampleRate / N;
    double psdScale = sampleRate * winSqSum;
    if (psdScale < 1e-15) psdScale = 1.0;

    QVector<double> freqs(halfN);
    QVector<double> psd(halfN);
    for (int i = 0; i < halfN; ++i) {
        freqs[i] = i * freqRes;
        double power = (re[i] * re[i] + im[i] * im[i]) / psdScale;
        psd[i] = 10.0 * std::log10(power + 1e-30);
    }

    return {freqs, psd};
}

int AdaptiveFFT::nextPowerOf2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return qMax(p, 4);
}

void AdaptiveFFT::inPlaceFFT(QVector<double>& real, QVector<double>& imag)
{
    int N = real.size();

    /* 位反转排列 */
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* Cooley-Tukey蝶形运算 */
    for (int len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = std::cos(angle);
        double wIm = std::sin(angle);

        for (int i = 0; i < N; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tRe = curRe * real[v] - curIm * imag[v];
                double tIm = curRe * imag[v] + curIm * real[v];
                real[v] = real[u] - tRe;
                imag[v] = imag[u] - tIm;
                real[u] += tRe;
                imag[u] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }
}

AdaptiveFFT::Stats AdaptiveFFT::stats() const { return m_stats; }

void AdaptiveFFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
