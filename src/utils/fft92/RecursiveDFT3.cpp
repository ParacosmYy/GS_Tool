#include "RecursiveDFT3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化递归DFT计算器
 * @param parent 父对象指针
 */
RecursiveDFT3::RecursiveDFT3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置DFT变换长度
 * @param size 变换长度，推荐为2的幂
 */
void RecursiveDFT3::setSize(int size)
{
    m_size = qMax(2, size);
}

/**
 * @brief 递归Cooley-Tukey FFT实现
 *
 * 将DFT分解为偶数和奇数索引的两个半长DFT，
 * 递归执行直到长度为1时直接返回。
 *
 * @param real 实部数组(输入/输出)
 * @param imag 虚部数组(输入/输出)
 */
static void recursiveDFT(QVector<double>& real, QVector<double>& imag)
{
    int N = real.size();
    if (N <= 1) return;

    /* 分解为偶数和奇数索引 */
    QVector<double> evenReal(N / 2), evenImag(N / 2);
    QVector<double> oddReal(N / 2), oddImag(N / 2);

    for (int i = 0; i < N / 2; ++i) {
        evenReal[i] = real[2 * i];
        evenImag[i] = imag[2 * i];
        oddReal[i] = real[2 * i + 1];
        oddImag[i] = imag[2 * i + 1];
    }

    /* 递归计算半长DFT */
    recursiveDFT(evenReal, evenImag);
    recursiveDFT(oddReal, oddImag);

    /* 合并结果 */
    for (int k = 0; k < N / 2; ++k) {
        double angle = -2.0 * M_PI * k / N;
        double wReal = std::cos(angle);
        double wImag = std::sin(angle);

        double tReal = wReal * oddReal[k] - wImag * oddImag[k];
        double tImag = wReal * oddImag[k] + wImag * oddReal[k];

        real[k] = evenReal[k] + tReal;
        imag[k] = evenImag[k] + tImag;
        real[k + N / 2] = evenReal[k] - tReal;
        imag[k + N / 2] = evenImag[k] - tImag;
    }
}

/**
 * @brief 对输入信号执行递归DFT计算
 *
 * 使用Cooley-Tukey递归算法计算快速傅里叶变换，
 * 返回频谱幅度。
 *
 * @param input 输入时域信号
 * @return 频谱幅度序列(前半部分)
 */
QVector<double> RecursiveDFT3::compute(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> spectrum;
    if (input.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computationCompleted(0);
        return spectrum;
    }

    int N = input.size();

    /* 补零到2的幂 */
    int p = 1;
    while (p < N) p *= 2;

    QVector<double> real(p, 0.0), imag(p, 0.0);
    for (int i = 0; i < N; ++i) real[i] = input[i];

    /* 执行递归FFT */
    recursiveDFT(real, imag);

    /* 计算幅度谱 */
    int halfN = p / 2;
    spectrum.resize(halfN);
    for (int k = 0; k < halfN; ++k) {
        spectrum[k] = std::sqrt(real[k] * real[k] + imag[k] * imag[k]) / N;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
    emit computationCompleted(spectrum.size());
    return spectrum;
}

/**
 * @brief 重置统计数据
 */
void RecursiveDFT3::resetStatistics()
{
    m_stats.totalComputations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
