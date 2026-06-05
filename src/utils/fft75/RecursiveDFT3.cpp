/**
 * @file RecursiveDFT3.cpp
 * @brief 递归离散傅里叶变换实现
 *
 * 基于Goertzel算法的递归DFT实现，支持逐样本
 * 滑动更新频谱。每个目标频率bin维护独立的递归
 * 状态变量(s0, s1, s2)，可实时获取幅度和相位信息。
 * 适用于实时频谱分析、音调检测和DTMF解码等场景。
 */

#include "utils/fft75/RecursiveDFT3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 初始FFT大小为0，目标bin列表为空。
 * 必须先调用initialize()设置参数后才能使用pushSample()。
 * 未初始化时pushSample()和magnitudeSpectrum()为空操作。
 */
RecursiveDFT3::RecursiveDFT3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 初始化DFT参数
 * @param fftSize DFT长度N(采样点数)
 * @param targetBins 目标频率bin索引列表(0到N-1)
 * @return true如果参数有效，false如果参数无效
 *
 * 为每个目标bin预分配三个Goertzel状态变量。
 * 每个bin k的递推系数: coeff = 2*cos(2*pi*k/N)
 * 初始化后所有状态归零，需要推入N个样本才能获得完整结果。
 */
bool RecursiveDFT3::initialize(int fftSize, const QVector<int>& targetBins)
{
    if (fftSize < 2 || targetBins.isEmpty()) return false;

    m_fftSize = fftSize;
    m_targetBins = targetBins;

    /* 初始化递归状态: 每个bin三个状态变量(s0, s1, s2) */
    /* 布局: m_state = [bin0_s0, bin0_s1, bin0_s2, bin1_s0, ...] */
    m_state.clear();
    m_state.resize(targetBins.size() * 3, 0.0);

    return true;
}

/**
 * @brief 推入新样本，递归更新频谱
 * @param sample 新的时域采样值
 *
 * 对每个目标bin执行Goertzel递推更新:
 *   s0[n] = x[n] + coeff * s1 - s2
 *   s2 = s1 (前一个s0)
 *   s1 = s0 (当前值)
 *
 * 递推保持计算状态，连续推入N个样本后，
 * 可通过magnitudeSpectrum()/phaseSpectrum()获取结果。
 * 复杂度O(K)，K为目标bin数，远小于FFT的O(N*logN)。
 */
void RecursiveDFT3::pushSample(double sample)
{
    if (m_targetBins.isEmpty() || m_fftSize == 0) return;

    for (int i = 0; i < m_targetBins.size(); ++i) {
        int k = m_targetBins[i];
        double w = 2.0 * M_PI * k / m_fftSize;
        double coeff = 2.0 * qCos(w);

        double& s0 = m_state[i * 3];
        double& s1 = m_state[i * 3 + 1];
        double& s2 = m_state[i * 3 + 2];

        /* Goertzel递推公式 */
        s0 = sample + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 更新采样计数 */
    m_stats.totalSamplesProcessed++;
}

/**
 * @brief 获取当前频谱幅度
 * @return 各目标bin的幅度值向量
 *
 * 从Goertzel递归状态计算复数DFT值并取模:
 *   Re = s1 - s2*cos(2*pi*k/N)
 *   Im = s2*sin(2*pi*k/N)
 *   |X[k]| = sqrt(Re^2 + Im^2) / (N/2)
 *
 * 归一化因子N/2使得单频正弦波的幅度等于其振幅。
 */
QVector<double> RecursiveDFT3::magnitudeSpectrum() const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> mag(m_targetBins.size(), 0.0);

    if (m_fftSize == 0 || m_targetBins.isEmpty()) return mag;

    for (int i = 0; i < m_targetBins.size(); ++i) {
        int k = m_targetBins[i];
        double w = 2.0 * M_PI * k / m_fftSize;

        /* 获取递归状态 */
        double s1 = m_state[i * 3 + 1];
        double s2 = m_state[i * 3 + 2];

        /* 从递归状态计算复数DFT值 */
        double re = s1 - s2 * qCos(w);
        double im = s2 * qSin(w);

        /* 幅度 = |X[k]| = sqrt(Re^2 + Im^2) */
        double magnitude = qSqrt(re * re + im * im);

        /* 归一化: 除以N/2得到与振幅对应的值 */
        mag[i] = magnitude / (m_fftSize / 2.0);
    }

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    if (m_stats.totalTransforms > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    }

    emit spectrumUpdated(m_targetBins.size());
    return mag;
}

/**
 * @brief 获取当前频谱相位
 * @return 各目标bin的相位值向量(弧度，范围[-pi, pi])
 *
 * 从复数DFT值计算相位角:
 *   Re = s1 - s2*cos(2*pi*k/N)
 *   Im = s2*sin(2*pi*k/N)
 *   phase = atan2(Im, Re)
 *
 * 相位信息可用于相位差分析、频率估计和系统辨识。
 */
QVector<double> RecursiveDFT3::phaseSpectrum() const
{
    QVector<double> phase(m_targetBins.size(), 0.0);

    if (m_fftSize == 0 || m_targetBins.isEmpty()) return phase;

    for (int i = 0; i < m_targetBins.size(); ++i) {
        int k = m_targetBins[i];
        double w = 2.0 * M_PI * k / m_fftSize;

        /* 获取递归状态 */
        double s1 = m_state[i * 3 + 1];
        double s2 = m_state[i * 3 + 2];

        /* 计算复数DFT值的实部和虚部 */
        double re = s1 - s2 * qCos(w);
        double im = s2 * qSin(w);

        /* 相位 = atan2(Im, Re) */
        phase[i] = qAtan2(im, re);
    }

    return phase;
}

/**
 * @brief 重置递归状态
 *
 * 清零所有Goertzel状态变量(s0, s1, s2)，
 * 相当于重新开始一个新的DFT计算周期。
 * 通常在处理完一个完整块(N个样本)后调用。
 */
void RecursiveDFT3::reset()
{
    std::fill(m_state.begin(), m_state.end(), 0.0);
}

/**
 * @brief 重置统计信息
 *
 * 清零所有累计统计数据(totalTransforms, totalSamplesProcessed)
 * 和计时累加器(m_timeSum)，平均处理时间归零。
 * 不影响递归计算状态(m_state)。
 */
void RecursiveDFT3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
