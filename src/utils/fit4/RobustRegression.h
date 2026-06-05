/**
 * @file RobustRegression.h
 * @brief 鲁棒回归 — RANSAC + Theil-Sen双重鲁棒估计
 *
 * 功能:
 *   - RANSAC: 随机抽样一致性，迭代寻找最佳内点集
 *   - Theil-Sen: 中位数斜率估计，对异常值高鲁棒
 *   - 支持线性回归 y = slope * x + intercept
 *   - 提供内点/外点分类、残差分析、R^2评估
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class RobustRegression
 * @brief 鲁棒回归引擎 — RANSAC与Theil-Sen双重策略
 *
 * RANSAC适用于大比例离群值场景，Theil-Sen适用于中等离群值场景。
 * 两者结合可提供更稳健的回归估计。
 */
class RobustRegression : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalFits = 0;               /**< 总拟合次数 */
        int totalInliers = 0;            /**< 总内点数 */
        int totalOutliers = 0;           /**< 总外点数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 回归结果 */
    struct RegressionResult {
        double slope = 0.0;              /**< 斜率 */
        double intercept = 0.0;          /**< 截距 */
        double rSquared = 0.0;           /**< R^2决定系数 */
        int inlierCount = 0;             /**< 内点数 */
        int outlierCount = 0;            /**< 外点数 */
        QVector<int> inlierIndices;      /**< 内点索引 */
        QVector<int> outlierIndices;     /**< 外点索引 */
        double inlierThreshold = 0.0;    /**< 使用的内点阈值 */
        bool valid = false;              /**< 结果是否有效 */
    };

    /** @brief 构造函数 */
    explicit RobustRegression(QObject* parent = nullptr);

    /**
     * @brief RANSAC线性回归
     * @param x X坐标数据
     * @param y Y坐标数据
     * @param threshold 内点判定阈值(残差绝对值)
     * @param maxIterations 最大迭代次数(0=自动)
     * @param confidence 置信度(0~1, 默认0.99)
     * @return 回归结果
     */
    RegressionResult fitRANSAC(const QVector<double>& x,
                                 const QVector<double>& y,
                                 double threshold = 0.0,
                                 int maxIterations = 0,
                                 double confidence = 0.99) const;

    /**
     * @brief Theil-Sen中位数回归
     * @param x X坐标数据
     * @param y Y坐标数据
     * @return 回归结果
     */
    RegressionResult fitTheilSen(const QVector<double>& x,
                                   const QVector<double>& y) const;

    /**
     * @brief 组合鲁棒回归: RANSAC拟合后再Theil-Sen精化
     * @param x X坐标数据
     * @param y Y坐标数据
     * @param threshold RANSAC内点阈值
     * @return 回归结果
     */
    RegressionResult fitRobust(const QVector<double>& x,
                                 const QVector<double>& y,
                                 double threshold = 0.0) const;

    /**
     * @brief 计算R^2决定系数
     * @param x X坐标
     * @param y Y坐标
     * @param slope 斜率
     * @param intercept 截距
     * @return R^2值
     */
    double computeRSquared(const QVector<double>& x,
                            const QVector<double>& y,
                            double slope, double intercept) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 */
    void fitCompleted(int inliers, int outliers, double rSquared);

private:
    /** @brief 用两点拟合直线 */
    QPair<double, double> fitLine(double x1, double y1,
                                   double x2, double y2) const;

    /** @brief 计算残差 */
    double residual(double x, double y,
                    double slope, double intercept) const;

    /** @brief 计算中位数 */
    double median(QVector<double> values) const;

    /** @brief 自动阈值(MAD估计) */
    double estimateThreshold(const QVector<double>& x,
                              const QVector<double>& y) const;

    mutable Stats m_stats;           /**< 统计信息 */
    mutable double m_timeSum = 0.0;  /**< 累计时间 */
};
