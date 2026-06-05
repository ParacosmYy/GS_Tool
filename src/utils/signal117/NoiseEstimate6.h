#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief NoiseEstimate6 - 噪声估计第6代实现
 *
 * 提供多种噪声估计算法，包括最小统计量法、分位数法、
 * 直方图法及改进的最小均方误差噪声功率谱估计。
 */
class NoiseEstimate6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEstimationOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit NoiseEstimate6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 使用最小统计量法估计噪声功率谱
     * @param powerSpectrum 输入功率谱序列
     * @return 估计的噪声功率谱
     */
    QVector<double> minStatistics(const QVector<QVector<double>>& powerSpectrum);

    /**
     * @brief 使用分位数法估计噪声
     * @param powerSpectrum 功率谱序列
     * @param quantile 分位数阈值 [0, 1]
     * @return 噪声功率估计值
     */
    double quantileEstimate(const QVector<double>& powerSpectrum, double quantile = 0.5);

    /**
     * @brief 在线逐帧噪声估计更新
     * @param currentFrame 当前帧功率谱
     * @return 更新后的噪声估计
     */
    QVector<double> updateOnline(const QVector<double>& currentFrame);

    /**
     * @brief 重置噪声估计状态
     */
    void resetState();

signals:
    void estimationCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
