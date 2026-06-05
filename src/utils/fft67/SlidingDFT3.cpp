/**
 * @file SlidingDFT3.cpp
 * @brief 滑动DFT（离散傅里叶变换）实现（第3版）
 *
 * 实现递归式滑动DFT，每输入一个新样本即可更新全部频率bin。
 * 通过旋转因子和递推公式避免完整FFT计算，时间复杂度O(1)每样本。
 * 支持阻尼因子防止数值累积误差。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft67/SlidingDFT3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化滑动DFT处理器
 * @param parent 父QObject对象指针
 */
SlidingDFT3::SlidingDFT3(QObject* parent)
    : QObject(parent)
{
    initTwiddleFactors();
}

/**
 * @brief 设置DFT大小
 * @param n 变换窗口大小，影响频率分辨率
 */
void SlidingDFT3::setSize(int n)
{
    m_n = qMax(2, n);
    m_X.resize(m_n, 0.0);
    m_Xprev.resize(m_n, 0.0);
    m_pos = 0;
    initTwiddleFactors();
}

/**
 * @brief 设置阻尼因子
 * @param damp 阻尼系数（0.0~1.0），用于防止数值不稳定
 */
void SlidingDFT3::setDamping(double damp)
{
    m_damp = qBound(0.0, damp, 1.0);
}

/**
 * @brief 初始化旋转因子
 *
 * 预计算每个频率bin的旋转因子 e^(j*2*pi*k/N)，
 * 用于递推更新DFT系数。
 */
void SlidingDFT3::initTwiddleFactors()
{
    m_twiddleRe.resize(m_n);
    m_twiddleIm.resize(m_n);

    for (int k = 0; k < m_n; ++k) {
        double angle = 2.0 * M_PI * k / m_n;
        m_twiddleRe[k] = qCos(angle);
        m_twiddleIm[k] = qSin(angle);
    }

    m_X.resize(m_n, 0.0);
    m_Xprev.resize(m_n, 0.0);
}

/**
 * @brief 推入一个新样本，更新所有DFT bin
 *
 * 递推公式：X_k[n] = damping * (e^(j*2*pi*k/N) * (X_k[n-1] + x[n] - x[n-N]))
 * 其中x[n-N]为N个样本前的输入值（通过Xprev间接获取）。
 *
 * @param sample 新输入的时域采样值
 */
void SlidingDFT3::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    for (int k = 0; k < m_n; ++k) {
        /* 递推更新DFT系数 */
        double newRe = m_damp * (m_twiddleRe[k] * (m_X[k] - m_Xprev[k])
                         + m_twiddleRe[k] * sample);
        double newIm = m_damp * (m_twiddleIm[k] * (m_X[k] - m_Xprev[k])
                         + m_twiddleIm[k] * sample);

        /* 简化的递推：X_k = damping * twiddle * (X_k + x_new - x_old) */
        double oldVal = m_X[k];
        double diff = sample - m_Xprev[k];

        m_X[k] = m_damp * (m_twiddleRe[k] * (m_X[k] + diff));
    }

    /* 记录当前样本用于下次递推 */
    m_Xprev[m_pos % m_n] = sample;
    m_pos++;

    /* 更新统计 */
    m_stats.totalUpdates++;
    m_stats.totalBins += m_n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit updated(m_pos % m_n);
}

/**
 * @brief 获取当前幅度谱
 * @return 每个频率bin的幅度值
 */
QVector<double> SlidingDFT3::spectrum() const
{
    QVector<double> mag(m_n);
    for (int k = 0; k < m_n; ++k) {
        mag[k] = qAbs(m_X[k]);
    }
    return mag;
}

/**
 * @brief 获取指定频率bin的幅度
 * @param k 频率bin索引（0~n-1）
 * @return 该bin的幅度值
 */
double SlidingDFT3::bin(int k) const
{
    if (k < 0 || k >= m_n) return 0.0;
    return qAbs(m_X[k]);
}

/**
 * @brief 获取当前统计信息
 * @return 更新统计结构
 */
SlidingDFT3::Stats SlidingDFT3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void SlidingDFT3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 重置所有DFT系数
 *
 * 将所有频率bin的DFT系数归零，等效于开始一个新窗口。
 * 在处理不连续的信号段时使用。
 */
void SlidingDFT3::resetCoefficients()
{
    m_X.fill(0.0);
    m_Xprev.fill(0.0);
    m_pos = 0;
}

/**
 * @brief 获取指定频率bin对应的实际频率
 * @param k bin索引
 * @return 对应的频率值（Hz），索引越界返回0
 */
double SlidingDFT3::binFrequency(int k) const
{
    if (k < 0 || k >= m_n) return 0.0;
    return static_cast<double>(k) * 44100.0 / m_n;
}

/**
 * @brief 获取幅度最大的频率bin索引
 *
 * 搜索所有频率bin，返回幅度最大的那个。
 * 可用于快速确定信号的主频分量。
 *
 * @return 最大幅度对应的bin索引
 */
int SlidingDFT3::peakBin() const
{
    int bestK = 0;
    double bestMag = 0.0;
    for (int k = 0; k < m_n; ++k) {
        double mag = qAbs(m_X[k]);
        if (mag > bestMag) {
            bestMag = mag;
            bestK = k;
        }
    }
    return bestK;
}

/**
 * @brief 获取峰值频率
 * @return 幅度最大的频率分量（Hz）
 */
double SlidingDFT3::peakFrequency() const
{
    return binFrequency(peakBin());
}
