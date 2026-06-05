/**
 * @file ConvolutionReverb.cpp
 * @brief 卷积混响实现 — FFT快速卷积+IR加载
 *
 * 使用FFT快速卷积实现脉冲响应混响:
 * - 重叠保留法(Overlap-Save)处理长信号
 * - 支持干湿比调节
 * - FFT/IFFT内部实现，无外部依赖
 *
 * 统计信息跟踪: 处理调用次数、采样点数、IR长度、平均耗时。
 */

#include "utils/dsp43/ConvolutionReverb.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
ConvolutionReverb::ConvolutionReverb(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率(Hz)
 */
void ConvolutionReverb::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置脉冲响应(IR)
 * @param ir 脉冲响应采样序列
 */
void ConvolutionReverb::setImpulseResponse(const QVector<double>& ir)
{
    m_ir = ir;

    // 设置块大小为不小于IR长度的最小2的幂
    m_blockSize = 1;
    while (m_blockSize < m_ir.size() * 2) {
        m_blockSize *= 2;
    }
    m_blockSize = qMax(m_blockSize, 256);

    // 初始化重叠缓冲区
    m_overlapBuffer.resize(m_blockSize);
    std::fill(m_overlapBuffer.begin(), m_overlapBuffer.end(), 0.0);

    m_stats.irLength = m_ir.size();
}

/**
 * @brief 设置干湿比
 * @param wet 湿信号比例 (0.0~1.0)
 * @param dry 干信号比例 (0.0~1.0)
 */
void ConvolutionReverb::setWetDry(double wet, double dry)
{
    m_wet = qBound(0.0, wet, 1.0);
    m_dry = qBound(0.0, dry, 1.0);
}

/**
 * @brief 处理输入信号，产生混响效果
 *
 * 使用重叠保留法(Overlap-Save)进行快速卷积:
 * 1. 将输入按块大小分段
 * 2. 对每块进行FFT卷积
 * 3. 叠加结果到输出
 * 4. 混合干湿信号
 *
 * @param input 输入采样序列
 * @return 混响处理后的采样序列
 */
QVector<double> ConvolutionReverb::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (m_ir.isEmpty() || input.isEmpty()) {
        return input;
    }

    const int inputLen = input.size();
    const int irLen = m_ir.size();
    const int outputLen = inputLen + irLen - 1;

    QVector<double> output(outputLen, 0.0);

    // 使用重叠相加法(Overlap-Add)
    const int blockLen = m_blockSize;
    const int numBlocks = (inputLen + blockLen - 1) / blockLen;

    for (int b = 0; b < numBlocks; ++b) {
        const int start = b * blockLen;
        const int end = qMin(start + blockLen, inputLen);

        // 提取当前块
        QVector<double> block(end - start, 0.0);
        for (int i = start; i < end; ++i) {
            block[i - start] = input[i];
        }

        // FFT卷积
        QVector<double> convResult = fftConvolve(block, m_ir);

        // 叠加到输出
        for (int i = 0; i < convResult.size() && (start + i) < outputLen; ++i) {
            output[start + i] += convResult[i];
        }
    }

    // 混合干湿信号
    QVector<double> result(inputLen, 0.0);
    for (int i = 0; i < inputLen; ++i) {
        result[i] = m_dry * input[i] + m_wet * output[i];
    }

    // 更新统计信息
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += inputLen;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit processingCompleted(inputLen, m_wet);
    return result;
}

/**
 * @brief 使用FFT进行快速卷积
 *
 * 卷积定理: a * b = IFFT(FFT(a) * FFT(b))
 *
 * @param a 输入信号块
 * @param b 脉冲响应
 * @return 卷积结果
 */
QVector<double> ConvolutionReverb::fftConvolve(const QVector<double>& a,
                                                const QVector<double>& b) const
{
    const int resultLen = a.size() + b.size() - 1;
    int fftLen = 1;
    while (fftLen < resultLen) {
        fftLen *= 2;
    }

    // 准备FFT输入
    QVector<double> realA(fftLen, 0.0), imagA(fftLen, 0.0);
    QVector<double> realB(fftLen, 0.0), imagB(fftLen, 0.0);

    for (int i = 0; i < a.size(); ++i) {
        realA[i] = a[i];
    }
    for (int i = 0; i < b.size(); ++i) {
        realB[i] = b[i];
    }

    // 正变换
    fft(realA, imagA);
    fft(realB, imagB);

    // 频域复数乘法
    for (int i = 0; i < fftLen; ++i) {
        double r = realA[i] * realB[i] - imagA[i] * imagB[i];
        double im = realA[i] * imagB[i] + imagA[i] * realB[i];
        realA[i] = r;
        imagA[i] = im;
    }

    // 逆变换
    ifft(realA, imagA);

    // 提取有效结果
    QVector<double> result(resultLen, 0.0);
    const double scale = 1.0 / fftLen;
    for (int i = 0; i < resultLen; ++i) {
        result[i] = realA[i] * scale;
    }

    return result;
}

/**
 * @brief 原地FFT变换(Cooley-Tukey)
 *
 * 使用迭代式基2 FFT算法。
 * 输入: real[]为实部, imag[]为虚部。
 * 输出: FFT结果覆写回同一数组。
 *
 * @param real 实部数组
 * @param imag 虚部数组
 */
void ConvolutionReverb::fft(QVector<double>& real, QVector<double>& imag) const
{
    const int n = real.size();
    if (n <= 1) return;

    // 位逆序置换
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

    // 蝶形运算
    for (int len = 2; len <= n; len <<= 1) {
        const double ang = -2.0 * M_PI / len;
        const double wReal = qCos(ang);
        const double wImag = qSin(ang);

        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;

                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];

                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;

                double newCurReal = curReal * wReal - curImag * wImag;
                double newCurImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
                curImag = newCurImag;
            }
        }
    }
}

/**
 * @brief 原地IFFT变换
 *
 * IFFT(x) = conj(FFT(conj(x))) / N
 *
 * @param real 实部数组
 * @param imag 虚部数组
 */
void ConvolutionReverb::ifft(QVector<double>& real, QVector<double>& imag) const
{
    const int n = real.size();

    // 共轭
    for (int i = 0; i < n; ++i) {
        imag[i] = -imag[i];
    }

    // 正向FFT
    fft(real, imag);

    // 共轭并除以N
    const double invN = 1.0 / n;
    for (int i = 0; i < n; ++i) {
        real[i] *= invN;
        imag[i] = -imag[i] * invN;
    }
}

/**
 * @brief 重置所有统计信息
 */
void ConvolutionReverb::resetStatistics()
{
    m_stats = Stats{};
    m_stats.irLength = m_ir.size();
    m_timeSum = 0.0;
}
