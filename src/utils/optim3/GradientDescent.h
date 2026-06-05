/**
 * @file GradientDescent.h
 * @brief 梯度下降优化器 — 支持动量/Adam自适应学习率
 *
 * 功能: 实现多种梯度下降变体，包括标准SGD、带动量的SGD、
 *       以及Adam自适应学习率优化器。支持自定义梯度和代价函数。
 *
 * 协作: DataInterpolator(参数拟合) / TrendPredictor(趋势预测)
 * @author Serial Tool Team
 * @date 2026-06-05
 */
#pragma once

#include <QObject>
#include <QVector>
#include <functional>
#include <QtGlobal>

/**
 * @class GradientDescent
 * @brief 梯度下降优化器
 *
 * 支持三种模式: 标准SGD、SGD+动量、Adam自适应学习率。
 * 用户需通过setGradientFunction()提供梯度计算回调，
 * 可选通过setCostFunction()设置代价函数用于收敛监测。
 */
class GradientDescent : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalOptimizations = 0;    ///< 累计优化次数
        double  avgIterations = 0.0;       ///< 平均迭代次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param learningRate 学习率(默认0.01)
     * @param momentum 动量系数(默认0.9)
     * @param parent 父对象
     */
    explicit GradientDescent(double learningRate = 0.01,
                             double momentum = 0.9,
                             QObject *parent = nullptr);

    /**
     * @brief 设置梯度计算函数
     * @param fn 梯度函数，输入参数向量，返回梯度向量
     */
    void setGradientFunction(
        std::function<QVector<double>(const QVector<double>&)> fn);

    /**
     * @brief 设置代价函数(可选，用于收敛监测)
     * @param fn 代价函数，输入参数向量，返回标量代价
     */
    void setCostFunction(
        std::function<double(const QVector<double>&)> fn);

    /**
     * @brief 执行优化
     * @param initialParams 初始参数向量
     * @param maxIterations 最大迭代次数
     * @return 优化后的参数向量
     */
    QVector<double> optimize(const QVector<double> &initialParams,
                             int maxIterations);

    /**
     * @brief 启用/禁用Adam优化器
     * @param enabled 是否启用Adam
     * @param beta1 一阶矩衰减率(默认0.9)
     * @param beta2 二阶矩衰减率(默认0.999)
     */
    void setAdam(bool enabled, double beta1 = 0.9, double beta2 = 0.999);

    /** @brief 设置学习率 @param lr 学习率 */
    void setLearningRate(double lr) { m_learningRate = qMax(1e-10, lr); }

    /** @brief 设置动量系数 @param m 动量系数 */
    void setMomentum(double m) { m_momentum = qBound(0.0, m, 1.0); }

    /** @brief 获取最后一次优化的代价值序列 @return 代价值列表 */
    QVector<double> costHistory() const { return m_costHistory; }

    /** @brief 获取统计信息 @return 常量引用 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 优化完成信号 @param iterations 实际迭代数 @param finalCost 最终代价 */
    void optimizationCompleted(int iterations, double finalCost);

private:
    double m_learningRate;      ///< 学习率
    double m_momentum;          ///< 动量系数
    bool m_adamEnabled;         ///< 是否使用Adam
    double m_beta1;             ///< Adam beta1
    double m_beta2;             ///< Adam beta2
    double m_epsilon;           ///< Adam数值稳定项

    std::function<QVector<double>(const QVector<double>&)> m_gradientFn; ///< 梯度函数
    std::function<double(const QVector<double>&)> m_costFn;              ///< 代价函数
    bool m_hasGradientFn;       ///< 是否已设置梯度函数

    QVector<double> m_costHistory;  ///< 最近一次优化的代价历史
    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 处理时间累加器
    quint64 m_iterationSum = 0;     ///< 迭代次数累加器
};
