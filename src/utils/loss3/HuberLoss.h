/**
 * @file HuberLoss.h
 * @brief Huber损失函数 — 鲁棒优化的平滑L1/L2混合损失
 *
 * 功能:
 *   - Huber损失: 当|residual|<=delta时为二次(L2), 否则为线性(L1)
 *   - 平滑过渡: 在delta处C1连续, 梯度有界
 *   - 支持批量计算、梯度计算、二阶导数(Hessian对角)
 *   - 自适应delta选择: 基于MAD估计
 *   - 适用于RANSAC内层优化、鲁棒回归、异常值容忍训练
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class HuberLoss
 * @brief Huber损失函数引擎 — 鲁棒优化的核心组件
 *
 * Huber损失在残差较小时等价于L2(平方)损失，保证收敛速度；
 * 残差较大时等价于L1(绝对值)损失，限制异常值的影响。
 * delta参数控制L1/L2的切换阈值。
 */
class HuberLoss : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalComputed = 0;          /**< 总计算次数 */
        int totalSamples = 0;           /**< 总样本数 */
        int totalOutliers = 0;          /**< 总超出delta的样本数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 损失计算结果 */
    struct LossResult {
        double loss = 0.0;              /**< 损失值 */
        double gradient = 0.0;          /**< 梯度 */
        double hessian = 0.0;           /**< 二阶导数(对角) */
        bool isOutlier = false;         /**< 是否为外点(|r|>delta) */
    };

    /** @brief 批量结果 */
    struct BatchResult {
        double totalLoss = 0.0;         /**< 总损失 */
        double avgLoss = 0.0;           /**< 平均损失 */
        double totalGradient = 0.0;     /**< 总梯度 */
        QVector<double> perSampleLoss;  /**< 逐样本损失 */
        QVector<double> perSampleGrad;  /**< 逐样本梯度 */
        int outlierCount = 0;           /**< 外点数 */
        int inlierCount = 0;            /**< 内点数 */
    };

    /** @brief 构造函数
     * @param delta Huber阈值(默认1.0)
     */
    explicit HuberLoss(double delta = 1.0, QObject* parent = nullptr);

    /**
     * @brief 计算单个样本的Huber损失
     * @param residual 残差值
     * @return 损失结果(含梯度/二阶导)
     */
    LossResult compute(double residual) const;

    /**
     * @brief 批量计算Huber损失
     * @param residuals 残差数组
     * @return 批量结果(含逐样本损失/梯度)
     */
    BatchResult computeBatch(const QVector<double>& residuals) const;

    /**
     * @brief 仅计算损失值(无梯度)
     * @param residual 残差值
     * @return Huber损失值
     */
    double loss(double residual) const;

    /**
     * @brief 计算梯度
     * @param residual 残差值
     * @return 梯度值
     */
    double gradient(double residual) const;

    /**
     * @brief 计算二阶导数(对角Hessian)
     * @param residual 残差值
     * @return 二阶导数值
     */
    double hessian(double residual) const;

    /**
     * @brief 自适应delta估计(基于MAD)
     * @param residuals 残差数组
     * @return 推荐的delta值
     */
    double estimateDelta(const QVector<double>& residuals) const;

    /**
     * @brief 从预测值和目标值计算
     * @param prediction 预测值
     * @param target 目标值
     * @return 损失结果
     */
    LossResult computeFromPrediction(double prediction, double target) const;

    /**
     * @brief 带权Huber损失(加权样本)
     * @param residuals 残差数组
     * @param weights 权重数组
     * @return 批量结果
     */
    BatchResult computeWeighted(const QVector<double>& residuals,
                                  const QVector<double>& weights) const;

    /** @brief 设置delta参数 */
    void setDelta(double delta);

    /** @brief 获取delta参数 */
    double delta() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 批量计算完成信号 */
    void batchComputed(int samples, double avgLoss, int outliers);

private:
    /** @brief 计算中位数 */
    double median(QVector<double> values) const;

    /** @brief 计算中位数绝对偏差(MAD) */
    double computeMAD(const QVector<double>& residuals) const;

    double m_delta;                    /**< Huber阈值 */
    mutable Stats m_stats;             /**< 统计信息 */
    mutable double m_timeSum = 0.0;    /**< 累计时间 */
};
