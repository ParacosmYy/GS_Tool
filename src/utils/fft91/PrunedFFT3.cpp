#include "PrunedFFT3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化剪枝FFT计算器
 * @param parent 父对象指针
 */
PrunedFFT3::PrunedFFT3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置剪枝掩码
 * @param mask 布尔数组，true表示需要计算对应的频率分量
 */
void PrunedFFT3::setPruneMask(const QVector<bool>& mask)
{
    m_pruneMask = mask;
}

/**
 * @brief 对输入信号执行剪枝FFT计算
 *
 * 根据剪枝掩码仅计算感兴趣的频率分量的DFT值，
 * 跳过不需要的频率索引以减少计算量。
 *
 * @param input 输入时域信号
 * @return 选定频率分量的频谱幅度
 */
QVector<double> PrunedFFT3::compute(const QVector<double>& input)
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

    const int N = input.size();

    /* 如果未设置掩码，默认计算全部频率 */
    if (m_pruneMask.isEmpty()) {
        m_pruneMask.resize(N, true);
    }

    /* 仅计算掩码为true的频率分量 */
    for (int k = 0; k < N && k < m_pruneMask.size(); ++k) {
        if (!m_pruneMask[k]) continue;

        double realPart = 0.0;
        double imagPart = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            realPart += input[n] * std::cos(angle);
            imagPart += input[n] * std::sin(angle);
        }
        spectrum.append(std::sqrt(realPart * realPart + imagPart * imagPart) / N);
    }

    /* 如果掩码比输入短，补充计算 */
    if (m_pruneMask.size() > N) {
        for (int k = N; k < m_pruneMask.size(); ++k) {
            if (m_pruneMask[k]) {
                spectrum.append(0.0);
            }
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
    emit computationCompleted(spectrum.size());
    return spectrum;
}

/**
 * @brief 获取当前掩码中需要计算的频率分量数量
 * @return 活跃频率分量数
 */
int PrunedFFT3::activeBinCount() const
{
    int count = 0;
    for (bool v : m_pruneMask) {
        if (v) count++;
    }
    return count;
}

/**
 * @brief 计算剪枝后的计算节省比例
 * @return 节省的计算量百分比(0.0~1.0)
 */
double PrunedFFT3::savingsRatio() const
{
    if (m_pruneMask.isEmpty()) return 0.0;
    int active = activeBinCount();
    return 1.0 - static_cast<double>(active) / m_pruneMask.size();
}

/**
 * @brief 设置全频段计算(重置掩码)
 */
void PrunedFFT3::setFullSpectrum()
{
    m_pruneMask.clear();
}

/**
 * @brief 获取计算效率提升比例
 * @return 相比全FFT节省的计算量百分比(0~1)
 */
double PrunedFFT3::efficiencyGain() const
{
    return savingsRatio();
}

/**
 * @brief 设置指定频率范围激活(便捷方法)
 * @param minFreq 最小频率索引
 * @param maxFreq 最大频率索引
 * @param totalBins 总频率bin数
 */
void PrunedFFT3::setFrequencyRange(int minFreq, int maxFreq, int totalBins)
{
    m_pruneMask.resize(totalBins, false);
    for (int i = minFreq; i <= maxFreq && i < totalBins; ++i) {
        m_pruneMask[i] = true;
    }
}

/**
 * @brief 重置统计数据
 */
void PrunedFFT3::resetStatistics()
{
    m_stats.totalComputations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
