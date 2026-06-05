/**
 * @file SpectrumAnalyzer.cpp
 * @brief 频谱分析引擎实现 — FFT/窗函数/峰值检测
 */

#include "utils/spectrum/SpectrumAnalyzer.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
SpectrumAnalyzer::SpectrumAnalyzer(QObject* parent)
    : QObject(parent)
    , m_sampleRate(1000.0)
    , m_windowFunc(WindowFunction::Hanning)
    , m_timeSum(0.0)
{
}

/** @brief 设置采样率 @param rate 采样率 */
void SpectrumAnalyzer::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置窗函数 @param window 窗函数 */
void SpectrumAnalyzer::setWindowFunction(WindowFunction window)
{
    m_windowFunc = window;
}

/** @brief 计算频谱 @param data 时域数据 @return (频率, 幅度) */
QPair<QVector<double>, QVector<double>> SpectrumAnalyzer::computeSpectrum(
    const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 4) {
        return {};
    }

    /* 补零到2的幂 */
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) {
        real[i] = data[i];
    }

    applyWindow(real);
    fft(real, imag);

    /* 计算幅度谱(前半部分) */
    int halfN = fftSize / 2;
    QVector<double> frequencies(halfN);
    QVector<double> magnitude(halfN);

    for (int i = 0; i < halfN; ++i) {
        frequencies[i] = static_cast<double>(i) * m_sampleRate / fftSize;
        double re = real[i], im = imag[i];
        magnitude[i] = 2.0 * qSqrt(re * re + im * im) / n;
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_stats.totalFFTsComputed++;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFFTsComputed);

    emit spectrumReady(n, halfN);
    return {frequencies, magnitude};
}

/** @brief 查找峰值 @param magnitude 幅度谱 @param threshold 阈值 @return 峰值列表 */
QList<SpectrumAnalyzer::Peak> SpectrumAnalyzer::findPeaks(
    const QVector<double>& magnitude, double threshold) const
{
    QList<Peak> peaks;
    if (magnitude.size() < 3) return peaks;

    double maxMag = *std::max_element(magnitude.begin(), magnitude.end());
    double threshVal = maxMag * threshold;

    for (int i = 1; i < magnitude.size() - 1; ++i) {
        if (magnitude[i] > magnitude[i - 1]
            && magnitude[i] > magnitude[i + 1]
            && magnitude[i] > threshVal) {
            Peak p;
            p.frequency = static_cast<double>(i) * m_sampleRate / (2.0 * magnitude.size());
            p.magnitude = magnitude[i];
            peaks.append(p);
        }
    }

    return peaks;
}

/** @brief 计算频谱质心 @param frequencies 频率 @param magnitude 幅度 @return 质心 */
double SpectrumAnalyzer::spectralCentroid(
    const QVector<double>& frequencies,
    const QVector<double>& magnitude) const
{
    if (frequencies.size() != magnitude.size() || frequencies.isEmpty()) {
        return 0.0;
    }

    double weightedSum = 0.0, totalWeight = 0.0;
    for (int i = 0; i < frequencies.size(); ++i) {
        weightedSum += frequencies[i] * magnitude[i];
        totalWeight += magnitude[i];
    }

    return (totalWeight > 0) ? weightedSum / totalWeight : 0.0;
}

/** @brief 重置统计 */
void SpectrumAnalyzer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 应用窗函数 @param data 数据 */
void SpectrumAnalyzer::applyWindow(QVector<double>& data)
{
    if (m_windowFunc == WindowFunction::None) return;

    int n = data.size();
    for (int i = 0; i < n; ++i) {
        double w = 1.0;
        double t = static_cast<double>(i) / static_cast<double>(n - 1);

        switch (m_windowFunc) {
        case WindowFunction::Hanning:
            w = 0.5 * (1.0 - qCos(2.0 * M_PI * t));
            break;
        case WindowFunction::Hamming:
            w = 0.54 - 0.46 * qCos(2.0 * M_PI * t);
            break;
        case WindowFunction::Blackman:
            w = 0.42 - 0.5 * qCos(2.0 * M_PI * t)
              + 0.08 * qCos(4.0 * M_PI * t);
            break;
        default:
            break;
        }
        data[i] *= w;
    }
}

/** @brief 基2 FFT @param real 实部 @param imag 虚部 */
void SpectrumAnalyzer::fft(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    imag.fill(0.0);

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* FFT蝶形运算 */
    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int even = i + j;
                int odd = i + j + len / 2;

                double tRe = curRe * real[odd] - curIm * imag[odd];
                double tIm = curRe * imag[odd] + curIm * real[odd];

                real[odd] = real[even] - tRe;
                imag[odd] = imag[even] - tIm;
                real[even] += tRe;
                imag[even] += tIm;

                double newRe = curRe * wRe - curIm * wIm;
                double newIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
                curIm = newIm;
            }
        }
    }
}
