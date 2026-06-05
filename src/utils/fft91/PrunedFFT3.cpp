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
 * @brief 重置统计数据
 */
void PrunedFFT3::resetStatistics()
{
    m_stats.totalComputations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
