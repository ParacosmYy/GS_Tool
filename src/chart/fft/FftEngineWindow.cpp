/**
 * @file FftEngineWindow.cpp
 * @brief FFT窗函数应用、频谱分析与统计接口实现
 *
 * 从FftEngine.cpp拆分而来，包含:
 *   - 四种窗函数的系数计算与原地应用(Rectangular/Hanning/Hamming/Blackman)
 *   - 单边幅度谱的计算(FFT复数输出→频率-幅度点集)
 *   - 统计计数器的查询与重置接口
 */

#include "chart/fft/FftEngine.h"

#include <QtMath>
#include <algorithm>

// ============================================================
// 窗函数
// ============================================================

/** @brief 对复数序列应用窗函数(原地修改) @param data 输入/输出复数序列 @param window 窗函数类型 */
void FftEngine::applyWindow(QVector<std::complex<double>>& data, WindowType window)
{
    const int N = data.size();
    if (N <= 1) {
        return;
    }

    switch (window) {
    case WindowType::Rectangular:
        break;

    case WindowType::Hanning:
        for (int n = 0; n < N; ++n) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (N - 1)));
            data[n] *= w;
        }
        break;

    case WindowType::Hamming:
        for (int n = 0; n < N; ++n) {
            double w = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (N - 1));
            data[n] *= w;
        }
        break;

    case WindowType::Blackman:
        for (int n = 0; n < N; ++n) {
            double w = 0.42
                     - 0.50 * qCos(2.0 * M_PI * n / (N - 1))
                     + 0.08 * qCos(4.0 * M_PI * n / (N - 1));
            data[n] *= w;
        }
        break;
    }
}

// ============================================================
// 单边幅度谱
// ============================================================

/** @brief 计算单边幅度谱(FFT输出→频率-幅度点集) @param fftResult FFT输出复数序列 @param sampleRate 采样率(Hz) @return 频率-幅度点集 */
QVector<QPointF> FftEngine::magnitudeSpectrum(
    const QVector<std::complex<double>>& fftResult,
    double sampleRate)
{
    const int N = fftResult.size();
    if (N <= 1) {
        return {};
    }

    const int halfN = N / 2;
    const double freqResolution = sampleRate / N;

    QVector<QPointF> spectrum;
    spectrum.reserve(halfN);

    for (int k = 0; k < halfN; ++k) {
        double magnitude = std::abs(fftResult[k]);

        if (k > 0 && k < halfN) {
            magnitude *= 2.0;
        }
        magnitude /= N;

        double freq = k * freqResolution;
        spectrum.append(QPointF(freq, magnitude));
    }

    return spectrum;
}

// ============================================================
// 统计接口
// ============================================================

/** @brief 获取总FFT变换执行次数 */
quint64 FftEngine::totalTransforms() const
{
    return m_totalTransforms;
}

/** @brief 获取总处理的采样点数（累计） */
quint64 FftEngine::totalSamplesProcessed() const
{
    return m_totalSamplesProcessed;
}

/** @brief 获取单次变换处理过的最大采样点数（峰值） */
quint64 FftEngine::maxSampleSize() const
{
    return m_maxSampleSize;
}

/** @brief 获取FFT计算中发生的错误次数 */
quint64 FftEngine::errorCount() const
{
    return m_errorCount;
}

/** @brief 重置所有统计计数器为初始值 */
void FftEngine::resetFftStatistics()
{
    m_totalTransforms = 0;
    m_totalTransformsExecuted = 0;
    m_totalSamplesProcessed = 0;
    m_maxSampleSize = 0;
    m_errorCount = 0;
}

/** @brief 重置所有统计计数器（别名接口） */
void FftEngine::resetStats()
{
    resetFftStatistics();
}
