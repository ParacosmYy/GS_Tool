#include "FastConv4.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class FastConv4
 * @brief 快速卷积处理器实现
 *
 * 基于重叠保留(Overlap-Save)方法的快速卷积。
 * 将长信号分成固定大小的块，每块与卷积核通过FFT
 * 在频域进行乘法，再IFFT回到时域。
 *
 * 复杂度: O(N * log(K))，远优于直接卷积的O(N*K)。
 * 其中N为信号长度，K为卷积核长度。
 *
 * 重叠保留法: 每块保留前(K-1)个重叠样本，取后半(L-K+1)个有效输出。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
FastConv4::FastConv4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 简化FFT(基2，就地)
 * @param real 实部数组
 * @param imag 虚部数组
 * @param inverse 是否逆变换
 */
static void simpleFFT(QVector<double>& real, QVector<double>& imag, bool inverse)
{
    int N = real.size();
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(real[i], real[j]); std::swap(imag[i], imag[j]); }
    }
    for (int len = 2; len <= N; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < N; i += len) {
            double cRe = 1.0, cIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tRe = cRe * real[v] - cIm * imag[v];
                double tIm = cRe * imag[v] + cIm * real[v];
                real[v] = real[u] - tRe; imag[v] = imag[u] - tIm;
                real[u] += tRe; imag[u] += tIm;
                double nRe = cRe * wRe - cIm * wIm;
                cIm = cRe * wIm + cIm * wRe; cRe = nRe;
            }
        }
    }
    if (inverse) { for (int i = 0; i < N; ++i) { real[i] /= N; imag[i] /= N; } }
}

/**
 * @brief 设置卷积核
 *
 * 将卷积核进行FFT预计算，存储频域表示。
 * FFT点数取大于(卷积核长度)的最小2的幂。
 *
 * @param kernel 卷积核(脉冲响应)
 */
void FastConv4::setKernel(const QVector<double>& kernel)
{
    QElapsedTimer timer;
    timer.start();

    m_kernel = kernel;

    /* 计算FFT大小 */
    int K = kernel.size();
    int fftSize = 1;
    while (fftSize < K) fftSize <<= 1;
    /* 重叠保留法需要至少2倍核长 */
    while (fftSize < 2 * K) fftSize <<= 1;

    /* 预计算卷积核的FFT */
    m_kernelFFTReal.resize(fftSize, 0.0);
    m_kernelFFTImag.resize(fftSize, 0.0);
    for (int i = 0; i < K; ++i) {
        m_kernelFFTReal[i] = kernel[i];
    }

    simpleFFT(m_kernelFFTReal, m_kernelFFTImag, false);

    /* 重置重叠缓冲区 */
    m_overlapBuffer.resize(K - 1, 0.0);

    m_timeSum += timer.elapsed();
}

/**
 * @brief 处理音频块(重叠保留法)
 *
 * 重叠保留法步骤:
 * 1. 将输入块前填充(K-1)个零或上一块的重叠数据
 * 2. FFT变换到频域
 * 3. 频域乘法: Y = X * H
 * 4. IFFT回到时域
 * 5. 丢弃前(K-1)个样本(循环卷积的不完整部分)
 * 6. 保留有效输出
 *
 * @param input 输入音频块
 * @return 卷积后的输出块
 */
QVector<double> FastConv4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;

    if (m_kernel.isEmpty() || input.isEmpty()) {
        m_timeSum += timer.elapsed();
        return output;
    }

    int K = m_kernel.size();
    int fftSize = m_kernelFFTReal.size();
    int validSamples = fftSize - K + 1; /* 每块有效输出采样数 */

    int inputPos = 0;
    while (inputPos < input.size()) {
        /* 构造FFT输入: 前(K-1)个重叠 + 当前块数据 */
        QVector<double> fftReal(fftSize, 0.0);
        QVector<double> fftImag(fftSize, 0.0);

        /* 填充重叠缓冲区 */
        for (int i = 0; i < m_overlapBuffer.size() && i < fftSize; ++i) {
            fftReal[i] = m_overlapBuffer[i];
        }

        /* 填充新数据 */
        int newDataStart = m_overlapBuffer.size();
        int newDataCount = qMin(validSamples, input.size() - inputPos);
        for (int i = 0; i < newDataCount; ++i) {
            fftReal[newDataStart + i] = input[inputPos + i];
        }
        inputPos += newDataCount;

        /* 保存重叠数据(末尾K-1个样本) */
        for (int i = 0; i < m_overlapBuffer.size(); ++i) {
            int srcIdx = fftSize - m_overlapBuffer.size() + i;
            if (srcIdx < fftReal.size()) {
                m_overlapBuffer[i] = fftReal[srcIdx];
            }
        }

        /* FFT */
        simpleFFT(fftReal, fftImag, false);

        /* 频域乘法 */
        for (int i = 0; i < fftSize; ++i) {
            double re = fftReal[i] * m_kernelFFTReal[i] - fftImag[i] * m_kernelFFTImag[i];
            double im = fftReal[i] * m_kernelFFTImag[i] + fftImag[i] * m_kernelFFTReal[i];
            fftReal[i] = re;
            fftImag[i] = im;
        }

        /* IFFT */
        simpleFFT(fftReal, fftImag, true);

        /* 取有效输出(跳过前K-1个样本) */
        for (int i = K - 1; i < fftSize; ++i) {
            output.append(fftReal[i]);
        }
    }

    /* 截断输出到输入长度 */
    if (output.size() > input.size() + K - 1) {
        output.resize(input.size() + K - 1);
    }

    m_stats.totalConvolutions++;
    m_stats.totalSamplesProcessed += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalConvolutions);

    emit convolutionCompleted(input.size(), m_kernel.size());

    return output;
}

/**
 * @brief 重置所有统计数据
 *
 * 将卷积计数、采样计数和计时归零。
 */
void FastConv4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_kernel.clear();
    m_overlapBuffer.clear();
    m_kernelFFTReal.clear();
    m_kernelFFTImag.clear();
}
