/**
 * @file ConstantQ3.cpp
 * @brief 常数Q变换实现（第3版）
 *
 * 实现常数Q变换（CQT），在频率轴上使用对数间隔的滤波器组。
 * 与标准FFT的线性频率分辨率不同，CQT在低频提供更精细的分辨率、
 * 在高频提供更粗的分辨率，更符合人耳对音高的感知。
 * 品质因子Q = f_k / delta_f 在所有频率bin上保持恒定。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft68/ConstantQ3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化CQT处理器
 * @param parent 父QObject对象指针
 */
ConstantQ3::ConstantQ3(QObject* parent)
    : QObject(parent)
{
    computeKernels();
}

/**
 * @brief 设置最低频率
 * @param fmin 最低分析频率（Hz）
 */
void ConstantQ3::setMinFreq(double fmin)
{
    m_fmin = qMax(1.0, fmin);
    computeKernels();
}

/**
 * @brief 设置最高频率
 * @param fmax 最高分析频率（Hz），不超过Nyquist频率
 */
void ConstantQ3::setMaxFreq(double fmax)
{
    m_fmax = qBound(m_fmin, fmax, m_sr / 2.0);
    computeKernels();
}

/**
 * @brief 设置每倍频程的频率bin数
 * @param bpo 每倍频程的bin数（典型值12, 24, 48）
 */
void ConstantQ3::setBinsPerOctave(int bpo)
{
    m_bpo = qMax(1, bpo);
    computeKernels();
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void ConstantQ3::setSampleRate(double sr)
{
    m_sr = qMax(1.0, sr);
    m_fmax = qMin(m_fmax, m_sr / 2.0);
    computeKernels();
}

/**
 * @brief 计算CQT核函数
 *
 * 根据参数计算每个频率bin对应的复指数核。
 * Q = 1 / (2^(1/bpo) - 1) 在所有bin上恒定。
 * 每个核的长度与该bin的周期成反比。
 */
void ConstantQ3::computeKernels()
{
    /* 计算Q因子 */
    m_Q = 1.0 / (qPow(2.0, 1.0 / m_bpo) - 1.0);

    /* 计算总bin数 */
    double octaves = qLn(m_fmax / m_fmin) / qLn(2.0);
    m_numBins = static_cast<int>(qCeil(octaves * m_bpo));
    m_numBins = qMax(1, m_numBins);

    /* 为每个bin生成复数核 */
    m_kernels.clear();
    m_kernels.resize(m_numBins);

    for (int k = 0; k < m_numBins; ++k) {
        /* 第k个bin的中心频率 */
        double fk = m_fmin * qPow(2.0, static_cast<double>(k) / m_bpo);

        /* 该bin对应的窗口长度 */
        int Nk = static_cast<int>(qCeil(m_Q * m_sr / fk));
        Nk = qMax(4, Nk);

        /* 构建复数核：Hann加窗的复指数 */
        m_kernels[k].resize(Nk);
        for (int n = 0; n < Nk; ++n) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (Nk - 1)));
            double angle = 2.0 * M_PI * m_Q * n / Nk;
            m_kernels[k][n] = w * qCos(angle); /* 仅存储实部 */
        }
    }
}

/**
 * @brief 执行前向常数Q变换
 *
 * 将输入信号与每个频率bin的核函数进行点积，
 * 得到对数间隔的频率表示。
 *
 * @param signal 输入时域信号
 * @return CQT系数（对数间隔的频率bin幅度）
 */
QVector<double> ConstantQ3::forward(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (signal.isEmpty() || m_kernels.isEmpty()) {
        emit transformCompleted(0, m_Q);
        return result;
    }

    result.resize(m_numBins, 0.0);
    int sigLen = signal.size();

    for (int k = 0; k < m_numBins; ++k) {
        int Nk = m_kernels[k].size();
        double re = 0.0, im = 0.0;

        for (int n = 0; n < Nk && n < sigLen; ++n) {
            /* 计算复数点积 */
            double angle = 2.0 * M_PI * m_Q * n / Nk;
            re += signal[n] * m_kernels[k][n];
            im += signal[n] * qSin(angle) * (0.5 * (1.0 - qCos(2.0 * M_PI * n / (Nk - 1))));
        }

        /* 幅度 */
        result[k] = qSqrt(re * re + im * im);

        /* 归一化 */
        result[k] /= Nk;
    }

    /* 更新统计 */
    m_stats.totalTransforms++;
    m_stats.totalBins += m_numBins;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(m_numBins, m_Q);
    return result;
}

/**
 * @brief 获取当前统计信息
 * @return 变换统计结构
 */
ConstantQ3::Stats ConstantQ3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void ConstantQ3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 获取每个频率bin的中心频率
 * @return bin中心频率数组（Hz）
 */
QVector<double> ConstantQ3::binFrequencies() const
{
    QVector<double> freqs(m_numBins);
    for (int k = 0; k < m_numBins; ++k) {
        freqs[k] = m_fmin * qPow(2.0, static_cast<double>(k) / m_bpo);
    }
    return freqs;
}

/**
 * @brief 获取频率分辨率
 *
 * 返回最低频率bin和最高频率bin的频率分辨率之比。
 * CQT的核心优势就是低频分辨率远高于高频。
 *
 * @return pair(最低分辨率Hz, 最高分辨率Hz)
 */
QPair<double, double> ConstantQ3::frequencyResolution() const
{
    if (m_numBins == 0) return {0.0, 0.0};

    double fLow = m_fmin;
    double fHigh = m_fmin * qPow(2.0, static_cast<double>(m_numBins - 1) / m_bpo);

    /* 频率分辨率 = f / Q */
    double resLow = fLow / m_Q;
    double resHigh = fHigh / m_Q;

    return {resLow, resHigh};
}

/**
 * @brief 获取最大核长度
 *
 * 最长核对应最低频率bin，决定了处理的最小信号长度。
 *
 * @return 最长核的采样点数
 */
int ConstantQ3::maxKernelLength() const
{
    int maxLen = 0;
    for (const auto& kernel : m_kernels) {
        maxLen = qMax(maxLen, kernel.size());
    }
    return maxLen;
}

/**
 * @brief 获取总核数（等于频率bin数）
 * @return 核数量
 */
int ConstantQ3::totalKernels() const
{
    return m_kernels.size();
}
