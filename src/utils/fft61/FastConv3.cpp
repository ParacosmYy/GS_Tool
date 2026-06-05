/**
 * @file FastConv3.cpp
 * @brief 快速卷积实现 (FFT卷积 + 分块处理)
 *
 * 实现基于FFT的快速卷积，支持三种模式和分块处理:
 * - "full": 完整卷积 (输出长度 = N + M - 1)
 * - "same": 等长卷积 (输出长度 = max(N, M))
 * - "valid": 有效卷积 (输出长度 = max(N, M) - min(N, M) + 1)
 * 对于长输入，使用overlap-add分块方法避免内存问题。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/fft61/FastConv3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化快速卷积器
 * @param parent 父QObject指针
 */
FastConv3::FastConv3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置卷积核(滤波器)
 * @param kernel 卷积核系数
 */
void FastConv3::setKernel(const QVector<double>& kernel)
{
    m_kernel = kernel;
}

/**
 * @brief 设置卷积模式
 * @param mode 卷积模式: "full" / "same" / "valid"
 */
void FastConv3::setMode(const QString& mode)
{
    if (mode == "full" || mode == "same" || mode == "valid") {
        m_mode = mode;
    }
}

/**
 * @brief 设置分块大小
 * @param size 分块处理的块大小 (默认 4096)
 *
 * 对于长输入信号，使用overlap-add分块方法
 */
void FastConv3::setBlockSize(int size)
{
    m_blockSize = qMax(256, size);
}

/**
 * @brief 对输入信号执行快速卷积
 *
 * 根据输入长度选择直接FFT卷积或分块卷积:
 * - 输入较短时直接计算FFT卷积
 * - 输入较长时分块处理
 *
 * @param input 输入信号
 * @return 卷积结果
 */
QVector<double> FastConv3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;

    if (input.isEmpty() || m_kernel.isEmpty()) {
        m_stats.totalConvolutions++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalConvolutions > 0)
            ? m_timeSum / m_stats.totalConvolutions : 0.0;
        emit convolutionCompleted(0, 0);
        return result;
    }

    const int N = input.size();
    const int M = m_kernel.size();

    /* 选择处理方式: 短信号直接FFT，长信号分块 */
    if (N + M - 1 <= m_blockSize * 2) {
        /* 直接FFT卷积 */
        result = fftConvolve(input, m_kernel);
    } else {
        /* 分块 overlap-add 卷积 */
        int fftSize = nextPow2(m_blockSize + M - 1);
        int blockLen = fftSize - M + 1;
        int fullLen = N + M - 1;

        QVector<double> output(fullLen, 0.0);

        /* 预计算核的FFT */
        QVector<double> paddedKernel(fftSize, 0.0);
        for (int i = 0; i < M; ++i) paddedKernel[i] = m_kernel[i];
        QVector<double> kernelFFT = realDFTForward(paddedKernel, fftSize);

        /* 逐块处理 */
        for (int start = 0; start < N; start += blockLen) {
            int end = qMin(start + blockLen, N);
            int blockActual = end - start;

            /* 零填充当前块 */
            QVector<double> block(fftSize, 0.0);
            for (int i = 0; i < blockActual; ++i) {
                block[i] = input[start + i];
            }

            /* FFT卷积当前块 */
            QVector<double> blockFFT = realDFTForward(block, fftSize);
            QVector<double> convFFT = complexMultiply(blockFFT, kernelFFT, fftSize);
            QVector<double> convBlock = realDFTInverse(convFFT, fftSize);

            /* Overlap-add到输出 */
            for (int i = 0; i < fftSize && start + i < fullLen; ++i) {
                output[start + i] += convBlock[i];
            }
        }

        result = output;
    }

    /* 根据模式截断结果 */
    int fullLen = N + M - 1;
    if (m_mode == "same") {
        int start = (fullLen - qMax(N, M)) / 2;
        int len = qMax(N, M);
        result = QVector<double>(result.begin() + start, result.begin() + qMin(start + len, result.size()));
    } else if (m_mode == "valid") {
        int validLen = qMax(N, M) - qMin(N, M) + 1;
        if (validLen > 0) {
            int start = qMin(N, M) - 1;
            result = QVector<double>(result.begin() + start, result.begin() + start + validLen);
        } else {
            result.clear();
        }
    }

    /* 更新统计 */
    m_stats.totalConvolutions++;
    m_stats.totalSamples += N;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConvolutions;

    emit convolutionCompleted(N, result.size());
    return result;
}

/**
 * @brief 重置所有统计数据
 */
void FastConv3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 使用FFT计算两个序列的完整卷积
 *
 * 步骤:
 * 1. 确定FFT大小 (>= N + M - 1 的2的幂)
 * 2. 零填充两个序列到FFT大小
 * 3. 分别计算FFT
 * 4. 频域相乘
 * 5. IFFT得到时域结果
 *
 * @param a 第一个序列
 * @param b 第二个序列
 * @return 卷积结果
 */
QVector<double> FastConv3::fftConvolve(const QVector<double>& a, const QVector<double>& b)
{
    const int N = a.size();
    const int M = b.size();
    int fftSize = nextPow2(N + M - 1);

    /* 零填充 */
    QVector<double> pa(fftSize, 0.0);
    QVector<double> pb(fftSize, 0.0);
    for (int i = 0; i < N; ++i) pa[i] = a[i];
    for (int i = 0; i < M; ++i) pb[i] = b[i];

    /* 前向FFT */
    QVector<double> fftA = realDFTForward(pa, fftSize);
    QVector<double> fftB = realDFTForward(pb, fftSize);

    /* 频域相乘 */
    QVector<double> fftProd = complexMultiply(fftA, fftB, fftSize);

    /* 逆FFT */
    QVector<double> result = realDFTInverse(fftProd, fftSize);

    /* 截取有效部分 */
    result.resize(N + M - 1);
    return result;
}

/**
 * @brief 计算大于等于n的最小2的幂
 * @param n 输入值
 * @return 2的幂值
 */
int FastConv3::nextPow2(int n) const
{
    if (n <= 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

/* === 辅助方法: 简化的DFT实现 === */

/**
 * @brief 实序列前向DFT，返回交织的[re0,im0,re1,im1,...]
 */
QVector<double> FastConv3::realDFTForward(const QVector<double>& x, int N) const
{
    /* 使用简化DFT (非FFT) 适用于中等规模 */
    QVector<double> X(N * 2, 0.0);
    for (int k = 0; k < N; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += x[n] * qCos(angle);
            im += x[n] * qSin(angle);
        }
        X[2 * k] = re;
        X[2 * k + 1] = im;
    }
    return X;
}

/**
 * @brief 逆DFT，输入为交织的[re0,im0,re1,im1,...]
 */
QVector<double> FastConv3::realDFTInverse(const QVector<double>& X, int N) const
{
    QVector<double> x(N, 0.0);
    for (int n = 0; n < N; ++n) {
        double val = 0.0;
        for (int k = 0; k < N; ++k) {
            double angle = 2.0 * M_PI * k * n / N;
            val += X[2 * k] * qCos(angle) - X[2 * k + 1] * qSin(angle);
        }
        x[n] = val / N;
    }
    return x;
}

/**
 * @brief 频域复数相乘
 */
QVector<double> FastConv3::complexMultiply(const QVector<double>& A, const QVector<double>& B, int N) const
{
    QVector<double> C(N * 2, 0.0);
    for (int k = 0; k < N; ++k) {
        double ar = A[2 * k], ai = A[2 * k + 1];
        double br = B[2 * k], bi = B[2 * k + 1];
        C[2 * k] = ar * br - ai * bi;
        C[2 * k + 1] = ar * bi + ai * br;
    }
    return C;
}
