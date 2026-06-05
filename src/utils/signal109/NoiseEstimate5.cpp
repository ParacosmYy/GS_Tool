#include "NoiseEstimate5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化噪声估计器
 * @param parent 父对象指针
 */
NoiseEstimate5::NoiseEstimate5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void NoiseEstimate5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_noiseSpectrum.clear();
    m_noiseFloorDb = -100.0;
}

/**
 * @brief 从功率谱估计噪声底
 *
 * 根据指定方法估计功率谱中的噪声底功率：
 * - MinimumStat: 取频谱最小值的滑动窗口最小统计量
 * - Percentile: 取频谱的指定分位数作为噪声底
 * - MovingAverage: 递归平均跟踪低能量段
 *
 * @param powerSpectrum 功率谱密度
 * @param method 估计方法
 * @return 噪声底功率(dB)
 */
double NoiseEstimate5::estimate(const QVector<double>& powerSpectrum, Method method)
{
    QElapsedTimer timer;
    timer.start();

    const int n = powerSpectrum.size();
    if (n == 0) {
        emit estimateCompleted(m_noiseFloorDb);
        return m_noiseFloorDb;
    }

    double noiseFloorLinear = 0.0;

    if (method == MinimumStat) {
        /* 最小统计量法：滑动窗口内取最小值 */
        const int winLen = qMax(1, n / 4);
        double globalMin = 1e18;
        for (int i = 0; i < n; ++i) {
            double val = qMax(powerSpectrum[i], 1e-20);
            if (val < globalMin) {
                globalMin = val;
            }
        }
        /* 对最小值进行偏差补偿 (Martin, 2001) */
        noiseFloorLinear = globalMin * 1.5;
    } else if (method == Percentile) {
        /* 分位数法：取15%分位数 */
        QVector<double> sorted = powerSpectrum;
        std::sort(sorted.begin(), sorted.end());
        int idx = qBound(0, static_cast<int>(n * 0.15), n - 1);
        noiseFloorLinear = qMax(sorted[idx], 1e-20);
    } else {
        /* 递归移动平均法 */
        double alpha = 0.95;
        if (m_noiseSpectrum.isEmpty()) {
            m_noiseSpectrum = powerSpectrum;
        }
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            double val = qMax(powerSpectrum[i], 1e-20);
            if (val < m_noiseSpectrum[i]) {
                m_noiseSpectrum[i] = alpha * m_noiseSpectrum[i]
                                     + (1.0 - alpha) * val;
            }
            sum += m_noiseSpectrum[i];
        }
        noiseFloorLinear = sum / n;
    }

    /* 转换为dB */
    m_noiseFloorDb = 10.0 * qLn(qMax(noiseFloorLinear, 1e-20)) / qLn(10.0);
    m_stats.lastNoiseFloorDb = m_noiseFloorDb;

    /* 初始化噪声谱（如果尚未初始化） */
    if (m_noiseSpectrum.isEmpty()) {
        m_noiseSpectrum.resize(n);
        for (int i = 0; i < n; ++i) {
            m_noiseSpectrum[i] = noiseFloorLinear;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEstimates++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    emit estimateCompleted(m_noiseFloorDb);
    return m_noiseFloorDb;
}

/**
 * @brief 计算信噪比
 *
 * SNR = signalPower - noiseFloorDb
 *
 * @param signalPower 信号功率(dB)
 * @param noiseFloorDb 噪声底(dB)
 * @return 信噪比(dB)
 */
double NoiseEstimate5::computeSNR(double signalPower, double noiseFloorDb) const
{
    return signalPower - noiseFloorDb;
}

/**
 * @brief 实时更新噪声估计
 *
 * 使用一阶递归平滑跟踪噪声功率谱，仅在能量低于当前估计时更新，
 * 对应MCRA算法中的噪声谱更新逻辑。
 *
 * @param newSpectrum 新的功率谱帧
 * @return 更新后的噪声底(dB)
 */
double NoiseEstimate5::updateOnline(const QVector<double>& newSpectrum)
{
    QElapsedTimer timer;
    timer.start();

    const int n = newSpectrum.size();
    if (n == 0) return m_noiseFloorDb;

    const double alpha = 0.92;

    if (m_noiseSpectrum.size() != n) {
        m_noiseSpectrum = newSpectrum;
    }

    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double val = qMax(newSpectrum[i], 1e-20);
        /* 仅在当前能量低于估计时快速跟踪 */
        if (val < m_noiseSpectrum[i]) {
            m_noiseSpectrum[i] = alpha * m_noiseSpectrum[i]
                                 + (1.0 - alpha) * val;
        } else {
            m_noiseSpectrum[i] = 0.99 * m_noiseSpectrum[i]
                                 + 0.01 * val;
        }
        sum += m_noiseSpectrum[i];
    }

    double noiseFloorLinear = sum / n;
    m_noiseFloorDb = 10.0 * qLn(qMax(noiseFloorLinear, 1e-20)) / qLn(10.0);
    m_stats.lastNoiseFloorDb = m_noiseFloorDb;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEstimates++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    return m_noiseFloorDb;
}
