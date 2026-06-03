/**
 * @file FftEngine.cpp
 * @brief FFT频谱计算引擎实现 -- Cooley-Tukey radix-2 DIT算法
 *
 * 实现 Cooley-Tukey 时间抽取（DIT）基-2 FFT算法，
 * 包含位反转置换和蝶形运算两个核心步骤。
 * 附带四种常用窗函数的实现。
 */

#include "chart/fft/FftEngine.h"

#include <QtMath>
#include <algorithm>

// ============================================================
// 构造 / 工具
// ============================================================

/** @brief 构造FFT引擎 @param parent 父对象 */
FftEngine::FftEngine(QObject* parent)
    : QObject(parent)
{
}

/** @brief 计算大于等于n的最小2的幂 @param n 输入值 @return >=n的最小2的幂 */
int FftEngine::nextPowerOf2(int n)
{
    if (n <= 0) {
        return 1;
    }
    if ((n & (n - 1)) == 0) {
        return n;
    }
    int highest = 0;
    while (n > 0) {
        n >>= 1;
        ++highest;
    }
    return (1 << highest);
}

/** @brief 计算以2为底的对数(整数部分) @param n 输入值(必须为2的幂) @return log2(n) */
int FftEngine::log2Int(int n)
{
    int bits = 0;
    while (n > 1) {
        n >>= 1;
        ++bits;
    }
    return bits;
}

/** @brief 计算位反转索引(蝶形运算前数据重排用) @param index 原始索引 @param bits 索引位数 @return 位反转后的索引 */
int FftEngine::bitReverse(int index, int bits)
{
    int reversed = 0;
    for (int i = 0; i < bits; ++i) {
        reversed = (reversed << 1) | (index & 1);
        index >>= 1;
    }
    return reversed;
}

/** @brief 将窗函数类型转换为可读字符串 @param window 窗函数类型 @return 窗函数英文名称 */
QString FftEngine::windowTypeName(WindowType window)
{
    switch (window) {
    case WindowType::Rectangular: return QStringLiteral("Rectangular");
    case WindowType::Hanning:     return QStringLiteral("Hanning");
    case WindowType::Hamming:     return QStringLiteral("Hamming");
    case WindowType::Blackman:    return QStringLiteral("Blackman");
    }
    return QStringLiteral("Unknown");
}

// ============================================================
// 主计算接口
// ============================================================

/** @brief 执行FFT频谱计算: 加窗→FFT→单边幅度谱 @param timeData 时域采样点(x=序号,y=采样值) @param sampleRate 采样率(Hz) @param window 窗函数类型 @param fftSize FFT长度(2的幂,0=自动) @return 频谱数据(x=频率Hz,y=幅度)，长度为fftSize/2 */
QVector<QPointF> FftEngine::compute(const QVector<QPointF>& timeData,
                                    double sampleRate,
                                    WindowType window,
                                    int fftSize)
{
    /* 无数据时返回空频谱 */
    if (timeData.isEmpty() || sampleRate <= 0.0) {
        ++m_errorCount;
        return {};
    }

    /* 确定FFT长度: 用户指定 or 自动取nextPowerOf2 */
    const int N = (fftSize > 0) ? nextPowerOf2(fftSize)
                                : nextPowerOf2(timeData.size());

    /* 构造复数序列，从时域数据的Y值提取 */
    QVector<std::complex<double>> data;
    data.reserve(N);
    for (int i = 0; i < N; ++i) {
        if (i < timeData.size()) {
            data.append(std::complex<double>(timeData[i].y(), 0.0));
        } else {
            data.append(std::complex<double>(0.0, 0.0));
        }
    }

    /* 1. 应用窗函数 */
    applyWindow(data, window);

    /* 2. 执行FFT（原地） */
    fftRadix2(data);

    /* 3. 计算单边幅度谱 */
    QVector<QPointF> spectrum = magnitudeSpectrum(data, sampleRate);

    /* 4. 查找基频（幅度最大处对应的频率） */
    double fundamentalFreq = 0.0;
    double maxMag = 0.0;
    for (const auto& pt : spectrum) {
        if (pt.y() > maxMag) {
            maxMag = pt.y();
            fundamentalFreq = pt.x();
        }
    }

    /* 更新统计计数器 */
    ++m_totalTransforms;
    ++m_totalTransformsExecuted;
    m_totalSamplesProcessed += static_cast<quint64>(timeData.size());
    quint64 nSample = static_cast<quint64>(timeData.size());
    if (nSample > m_maxSampleSize) {
        m_maxSampleSize = nSample;
    }

    emit spectrumComputed(spectrum, fundamentalFreq);
    return spectrum;
}

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
// Cooley-Tukey radix-2 DIT FFT
// ============================================================

/** @brief 执行Cooley-Tukey radix-2 DIT FFT原地计算 @param data 输入/输出复数序列(长度必须为2的幂) */
void FftEngine::fftRadix2(QVector<std::complex<double>>& data)
{
    const int N = data.size();
    if (N <= 1) {
        return;
    }

    const int stages = log2Int(N);

    /* 步骤1: 位反转置换 */
    for (int i = 0; i < N; ++i) {
        int j = bitReverse(i, stages);
        if (j > i) {
            std::swap(data[i], data[j]);
        }
    }

    /* 步骤2: 蝶形运算 */
    for (int stage = 1; stage <= stages; ++stage) {
        int span = (1 << stage);
        int halfSpan = span >> 1;

        double angleStep = -2.0 * M_PI / span;
        double wReal = qCos(angleStep);
        double wImag = qSin(angleStep);

        for (int base = 0; base < N; base += span) {
            double curReal = 1.0;
            double curImag = 0.0;

            for (int k = 0; k < halfSpan; ++k) {
                int top = base + k;
                int bot = top + halfSpan;

                double tReal = curReal * data[bot].real() - curImag * data[bot].imag();
                double tImag = curReal * data[bot].imag() + curImag * data[bot].real();

                data[bot] = std::complex<double>(
                    data[top].real() - tReal,
                    data[top].imag() - tImag);
                data[top] = std::complex<double>(
                    data[top].real() + tReal,
                    data[top].imag() + tImag);

                double newReal = curReal * wReal - curImag * wImag;
                double newImag = curReal * wImag + curImag * wReal;
                curReal = newReal;
                curImag = newImag;
            }
        }
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
