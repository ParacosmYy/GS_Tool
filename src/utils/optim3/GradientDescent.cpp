/**
 * @file GradientDescent.cpp
 * @brief 梯度下降优化器实现 — 标准SGD/动量/Adam
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/optim3/GradientDescent.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数
 *  @param learningRate 学习率(默认0.01)
 *  @param momentum 动量系数(默认0.9)
 *  @param parent 父对象 */
GradientDescent::GradientDescent(double learningRate, double momentum,
                                 QObject *parent)
    : QObject(parent)
    , m_learningRate(qMax(1e-10, learningRate))
    , m_momentum(qBound(0.0, momentum, 1.0))
    , m_adamEnabled(false)
    , m_beta1(0.9)
    , m_beta2(0.999)
    , m_epsilon(1e-8)
    , m_hasGradientFn(false)
{
}

/** @brief 设置梯度计算函数
 *  @param fn 梯度函数，输入参数向量返回梯度向量 */
void GradientDescent::setGradientFunction(
    std::function<QVector<double>(const QVector<double>&)> fn)
{
    m_gradientFn = fn;
    m_hasGradientFn = static_cast<bool>(fn);
}

/** @brief 设置代价函数(可选，用于收敛监测)
 *  @param fn 代价函数 */
void GradientDescent::setCostFunction(
    std::function<double(const QVector<double>&)> fn)
{
    m_costFn = fn;
}

/** @brief 执行梯度下降优化
 *  @param initialParams 初始参数向量
 *  @param maxIterations 最大迭代次数
 *  @return 优化后的参数向量 */
QVector<double> GradientDescent::optimize(const QVector<double> &initialParams,
                                          int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_hasGradientFn || initialParams.isEmpty() || maxIterations <= 0) {
        return QVector<double>(initialParams);
    }

    int dim = initialParams.size();
    QVector<double> params = initialParams;
    m_costHistory.clear();

    /* 动量项初始化 */
    QVector<double> velocity(dim, 0.0);

    /* Adam矩估计初始化 */
    QVector<double> mAdam(dim, 0.0);  ///< 一阶矩(均值)
    QVector<double> vAdam(dim, 0.0);  ///< 二阶矩(方差)

    double beta1t = 1.0; /* beta1^t，用于偏差修正 */
    double beta2t = 1.0; /* beta2^t */

    for (int iter = 0; iter < maxIterations; ++iter) {
        /* 计算梯度 */
        QVector<double> grad = m_gradientFn(params);
        if (grad.size() != dim) break;

        /* 记录代价(如果设置了代价函数) */
        if (m_costFn) {
            double cost = m_costFn(params);
            m_costHistory.append(cost);
        }

        if (m_adamEnabled) {
            /* Adam更新规则 */
            beta1t *= m_beta1;
            beta2t *= m_beta2;

            for (int j = 0; j < dim; ++j) {
                mAdam[j] = m_beta1 * mAdam[j] + (1.0 - m_beta1) * grad[j];
                vAdam[j] = m_beta2 * vAdam[j] + (1.0 - m_beta2) * grad[j] * grad[j];

                /* 偏差修正 */
                double mHat = mAdam[j] / (1.0 - beta1t);
                double vHat = vAdam[j] / (1.0 - beta2t);

                params[j] -= m_learningRate * mHat / (qSqrt(vHat) + m_epsilon);
            }
        } else {
            /* 标准SGD + 动量 */
            for (int j = 0; j < dim; ++j) {
                velocity[j] = m_momentum * velocity[j] - m_learningRate * grad[j];
                params[j] += velocity[j];
            }
        }
    }

    /* 最终代价记录 */
    if (m_costFn) {
        m_costHistory.append(m_costFn(params));
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalOptimizations;
    m_iterationSum += static_cast<quint64>(maxIterations);
    m_stats.avgIterations = static_cast<double>(m_iterationSum)
        / static_cast<double>(m_stats.totalOptimizations);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOptimizations);

    double finalCost = m_costHistory.isEmpty() ? 0.0 : m_costHistory.last();
    emit optimizationCompleted(maxIterations, finalCost);
    return params;
}

/** @brief 启用/禁用Adam优化器
 *  @param enabled 是否启用Adam
 *  @param beta1 一阶矩衰减率(默认0.9)
 *  @param beta2 二阶矩衰减率(默认0.999) */
void GradientDescent::setAdam(bool enabled, double beta1, double beta2)
{
    m_adamEnabled = enabled;
    m_beta1 = qBound(0.0, beta1, 1.0);
    m_beta2 = qBound(0.0, beta2, 1.0);
}

/** @brief 重置统计计数器 */
void GradientDescent::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_iterationSum = 0;
}
