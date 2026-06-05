/**
 * @file BoxCounting.h
 * @brief 盒计数法(Box-Counting)分形维数计算
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class BoxCounting
 * @brief 盒计数分形维数计算器
 *
 * 通过不同尺度的盒子覆盖来估计分形维数D = -lim(log(N)/log(s))。
 * 支持二维点集和灰度图像的分形维数计算。
 */
class BoxCounting : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalComputations = 0;  /**< 总计算次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit BoxCounting(QObject* parent = nullptr);

    /**
     * @brief 计算2D点集的分形维数
     * @param points 点集[(x,y), ...]
     * @param minBoxSize 最小盒子尺寸
     * @param maxBoxSize 最大盒子尺寸
     * @param steps 尺度数
     * @return (维数, R²拟合优度)
     */
    QPair<double, double> computeDimension(
        const QVector<QPair<double, double>>& points,
        double minBoxSize, double maxBoxSize, int steps = 10) const;

    /**
     * @brief 计算灰度图像的分形维数
     * @param image 二维灰度值矩阵
     * @param threshold 灰度阈值(高于此值的像素视为前景)
     * @param steps 尺度数
     * @return (维数, R²)
     */
    QPair<double, double> computeImageDimension(
        const QVector<QVector<double>>& image,
        double threshold = 128.0, int steps = 8) const;

    /**
     * @brief 获取不同尺度下的盒子计数
     * @param points 点集
     * @param scales 尺度列表
     * @return [(log(1/s), log(N)), ...]
     */
    QVector<QPair<double, double>> boxCounts(
        const QVector<QPair<double, double>>& points,
        const QVector<double>& scales) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void computationCompleted(double dimension, double rSquared);

private:
    int countBoxes(const QVector<QPair<double, double>>& points, double boxSize) const;
    QPair<double, double> linearRegression(const QVector<QPair<double, double>>& data) const;

    mutable Stats m_stats;
    mutable double m_timeSum;
};
