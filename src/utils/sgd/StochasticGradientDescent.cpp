/**
 * @file StochasticGradientDescent.cpp
 * @brief 随机梯度下降优化器实现 — SGD/Momentum/Nesterov/Adam
 */

#include "utils/sgd/StochasticGradientDescent.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <functional>

/** @brief 构造函数 @param parent 父对象 */
StochasticGradientDescent::StochasticGradientDescent(QObject* parent)
    : QObject(parent)
    , m_config()
{
}

/** @brief 设置优化器类型 @param type 优化器类型 */
void StochasticGradientDescent::setOptimizerType(OptimizerType type)
{
    m_type = type;
    resetState();
}

/** @brief 设置配置参数 @param config 配置结构体 */
void StochasticGradientDescent::setConfig(const Config& config)
{
    m_config = config;
    resetState();
}

/** @brief 执行一步梯度更新
 *  @param parameters 当前参数
 *  @param gradients 当前梯度
 *  @return 更新后的参数 */
QVector<double> StochasticGradientDescent::step(
    const QVector<double>& parameters,
    const QVector<double>& gradients)
{
    if (parameters.size() != gradients.size() || parameters.isEmpty()) {
        return parameters;
    }

    QVector<double> result;
    switch (m_type) {
    case OptimizerType::VanillaSGD:
        result = updateVanilla(parameters, gradients);
        break;
    case OptimizerType::Momentum:
        result = updateMomentum(parameters, gradients);
        break;
    case OptimizerType::Nesterov:
        result = updateNesterov(parameters, gradients);
        break;
    case OptimizerType::Adam:
        result = updateAdam(parameters, gradients);
        break;
    }

    ++m_stats.totalUpdates;
    return result;
}

/** @brief 完整优化过程
 *  @param initialParams 初始参数
 *  @param lossGradient 损失函数梯度回调
 *  @return 优化结果 */
StochasticGradientDescent::OptimizeResult
StochasticGradientDescent::optimize(
    const QVector<double>& initialParams,
    std::function<QVector<double>(const QVector<double>&)> lossGradient)
{
    QElapsedTimer timer;
    timer.start();

    resetState();
    OptimizeResult result;
    result.parameters = initialParams;

    QVector<double> params = initialParams;
    double prevLossNorm = 1e18;

    for (int iter = 0; iter < m_config.maxIterations; ++iter) {
        /* 计算梯度 */
        QVector<double> grads = lossGradient(params);
        if (grads.size() != params.size()) break;

        /* 计算梯度范数用于收敛判断 */
        double gradNorm = 0.0;
        for (double g : grads) {
            gradNorm += g * g;
        }
        gradNorm = qSqrt(gradNorm);

        /* 更新参数 */
        params = step(params, grads);

        /* 收敛检查 */
        if (qAbs(prevLossNorm - gradNorm) < m_config.tolerance) {
            result.converged = true;
            result.iterations = iter + 1;
            break;
        }
        prevLossNorm = gradNorm;

        emit parameterUpdated(iter, gradNorm);
    }

    if (!result.converged) {
        result.iterations = m_config.maxIterations;
    }

    result.parameters = params;
    result.finalLoss = prevLossNorm;

    /* 更新统计 */
    ++m_stats.totalOptimizations;
    if (result.converged) {
        ++m_stats.totalConverged;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOptimizations);

    emit optimizationFinished(result);
    return result;
}

/** @brief 重置优化器内部状态 */
void StochasticGradientDescent::resetState()
{
    m_velocity.clear();
    m_momentFirst.clear();
    m_momentSecond.clear();
    m_timeStep = 0;
}

/** @brief 获取统计信息 */
StochasticGradientDescent::Stats StochasticGradientDescent::stats() const
{
    return m_stats;
}

/** @brief 重置统计 */
void StochasticGradientDescent::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Vanilla SGD更新
 *  @param params 当前参数
 *  @param grads 当前梯度
 *  @return 更新后的参数 */
QVector<double> StochasticGradientDescent::updateVanilla(
    const QVector<double>& params,
    const QVector<double>& grads)
{
    QVector<double> result(params.size());
    double lr = m_config.learningRate;
    double wd = m_config.weightDecay;

    for (int i = 0; i < params.size(); ++i) {
        /* 带L2正则化的梯度: g + wd * w */
        double grad = grads[i] + wd * params[i];
        result[i] = params[i] - lr * grad;
    }
    return result;
}

/** @brief Momentum SGD更新
 *  @param params 当前参数
 *  @param grads 当前梯度
 *  @return 更新后的参数 */
QVector<double> StochasticGradientDescent::updateMomentum(
    const QVector<double>& params,
    const QVector<double>& grads)
{
    if (m_velocity.size() != params.size()) {
        m_velocity.resize(params.size(), 0.0);
    }

    QVector<double> result(params.size());
    double lr = m_config.learningRate;
    double mu = m_config.momentum;
    double wd = m_config.weightDecay;

    for (int i = 0; i < params.size(); ++i) {
        double grad = grads[i] + wd * params[i];
        /* v = μ * v + g */
        m_velocity[i] = mu * m_velocity[i] + grad;
        /* w = w - lr * v */
        result[i] = params[i] - lr * m_velocity[i];
    }
    return result;
}

/** @brief Nesterov加速梯度更新
 *  @param params 当前参数
 *  @param grads 当前梯度
 *  @return 更新后的参数 */
QVector<double> StochasticGradientDescent::updateNesterov(
    const QVector<double>& params,
    const QVector<double>& grads)
{
    if (m_velocity.size() != params.size()) {
        m_velocity.resize(params.size(), 0.0);
    }

    QVector<double> result(params.size());
    double lr = m_config.learningRate;
    double mu = m_config.momentum;
    double wd = m_config.weightDecay;

    for (int i = 0; i < params.size(); ++i) {
        double grad = grads[i] + wd * params[i];
        double prevVel = m_velocity[i];
        /* v = μ * v + g */
        m_velocity[i] = mu * m_velocity[i] + grad;
        /* w = w - lr * (μ * v_new + g) 等价形式 */
        result[i] = params[i] - lr * (mu * m_velocity[i] + grad)
                    + mu * prevVel;
    }
    return result;
}

/** @brief Adam优化器更新
 *  @param params 当前参数
 *  @param grads 当前梯度
 *  @return 更新后的参数 */
QVector<double> StochasticGradientDescent::updateAdam(
    const QVector<double>& params,
    const QVector<double>& grads)
{
    int n = params.size();
    if (m_momentFirst.size() != n) {
        m_momentFirst.resize(n, 0.0);
        m_momentSecond.resize(n, 0.0);
        m_timeStep = 0;
    }

    ++m_timeStep;
    QVector<double> result(n);
    double lr = m_config.learningRate;
    double b1 = m_config.beta1;
    double b2 = m_config.beta2;
    double eps = m_config.epsilon;
    double wd = m_config.weightDecay;

    for (int i = 0; i < n; ++i) {
        double grad = grads[i] + wd * params[i];

        /* 一阶矩估计: m = β1 * m + (1-β1) * g */
        m_momentFirst[i] = b1 * m_momentFirst[i] + (1.0 - b1) * grad;

        /* 二阶矩估计: v = β2 * v + (1-β2) * g^2 */
        m_momentSecond[i] = b2 * m_momentSecond[i] + (1.0 - b2) * grad * grad;

        /* 偏差修正 */
        double mHat = m_momentFirst[i] / (1.0 - qPow(b1, m_timeStep));
        double vHat = m_momentSecond[i] / (1.0 - qPow(b2, m_timeStep));

        /* 参数更新: w = w - lr * mHat / (sqrt(vHat) + eps) */
        result[i] = params[i] - lr * mHat / (qSqrt(vHat) + eps);
    }
    return result;
}
