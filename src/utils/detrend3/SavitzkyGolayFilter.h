/**
 * @file SavitzkyGolayFilter.h
 * @brief Savitzky-Golay平滑滤波器
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Savitzky-Golay平滑微分滤波器
 *
 * 通过局部多项式拟合实现数据平滑和微分,
 * 广泛用于光谱分析和信号处理。
 */
class SavitzkyGolayFilter : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalFiltered = 0;          ///< 总滤波次数
        int totalDerivatives = 0;       ///< 总微分次数
        double avgProcessingTimeMs = 0.0;
    };

    explicit SavitzkyGolayFilter(QObject* parent = nullptr);

    /**
     * @brief 平滑滤波
     * @param data 输入数据
     * @param windowSize 窗口大小(奇数)
     * @param polyOrder 多项式阶数
     * @return 平滑后数据
     */
    QVector<double> smooth(const QVector<double>& data,
                            int windowSize, int polyOrder);

    /**
     * @brief 一阶微分滤波
     * @param data 输入数据
     * @param windowSize 窗口大小
     * @param polyOrder 多项式阶数
     * @param dx 采样间隔
     * @return 一阶导数
     */
    QVector<double> derivative(const QVector<double>& data,
                                int windowSize, int polyOrder,
                                double dx = 1.0);

    /**
     * @brief 二阶微分滤波
     */
    QVector<double> secondDerivative(const QVector<double>& data,
                                      int windowSize, int polyOrder,
                                      double dx = 1.0);

    /**
     * @brief 计算SG卷积系数
     * @param windowSize 窗口大小
     * @param polyOrder 多项式阶数
     * @param deriv 微分阶数
     * @return 卷积系数
     */
    QVector<double> computeCoeffs(int windowSize, int polyOrder,
                                   int deriv = 0, double dx = 1.0);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 滤波完成信号 */
    void filtered(int dataSize, int windowSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<double> applyConvolution(const QVector<double>& data,
                                      const QVector<double>& coeffs);
};
