/**
 * @file AdaptiveFilter2.cpp
 * @brief 自适应滤波器实现 — LMS/NLMS/SignLMS + 收敛性追踪
 */

#include "utils/dsp13/AdaptiveFilter2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/* ── 构造/配置 ── */

/** @brief 构造函数 @param parent 父对象 */
AdaptiveFilter2::AdaptiveFilter2(QObject* parent)
    : QObject(parent)
    , m_algorithm(Algorithm::NLMS)
    , m_order(32)
    , m_mu(0.1)
    , m_epsilon(1e-6)
    , m_convThreshold(0.01)
    , m_bufferIndex(0)
    , m_errorSum(0.0)
    , m_errorCount(0)
    , m_minSteadyError(1e300)
{
    reset();
}

/** @brief 设置算法类型 @param algo 算法 */
void AdaptiveFilter2::setAlgorithm(Algorithm algo)
{
    m_algorithm = algo;
}

/** @brief 设置滤波器阶数 @param order 阶数 */
void AdaptiveFilter2::setFilterOrder(int order)
{
    m_order = qMax(1, order);
    reset();
}

/** @brief 设置步长参数 @param mu 步长 */
void AdaptiveFilter2::setStepSize(double mu)
{
    m_mu = qBound(1e-6, mu, 2.0);
}

/** @brief 设置NLMS正则化因子 @param eps 因子 */
void AdaptiveFilter2::setRegularization(double eps)
{
    m_epsilon = qMax(1e-12, eps);
}

/** @brief 设置收敛判定阈值 @param threshold 阈值 */
void AdaptiveFilter2::setConvergenceThreshold(double threshold)
{
    m_convThreshold = qMax(1e-10, threshold);
}

/* ── 核心滤波 ── */

/** @brief 单步自适应滤波 @param input 输入信号 @param desired 期望信号 @return 步骤结果 */
AdaptiveFilter2::StepResult AdaptiveFilter2::processStep(double input,
                                                          double desired)
{
    QElapsedTimer timer;
    timer.start();

    StepResult result;

    /* 将新样本推入延迟线 */
    m_inputBuffer[m_bufferIndex] = input;

    /* 计算滤波器输出: y = w^T * x */
    double output = 0.0;
    for (int i = 0; i < m_order; ++i) {
        int idx = (m_bufferIndex - i + m_order) % m_order;
        output += m_weights[i] * m_inputBuffer[idx];
    }

    /* 计算误差 */
    double error = desired - output;

    /* 更新权重 */
    switch (m_algorithm) {
    case Algorithm::LMS:
        updateLMS(error);
        break;
    case Algorithm::NLMS:
        updateNLMS(error);
        break;
    case Algorithm::SignLMS:
        updateSignLMS(error);
        break;
    }

    /* 推进缓冲区索引 */
    m_bufferIndex = (m_bufferIndex + 1) % m_order;

    /* 收敛追踪 */
    m_errorSum += error * error;
    ++m_errorCount;
    checkConvergence();

    /* 记录峰值误差 */
    double absErr = std::abs(error);
    if (absErr > m_stats.peakError) {
        m_stats.peakError = absErr;
    }

    result.output = output;
    result.error = error;
    result.weights = m_weights;

    /* 更新统计 */
    ++m_stats.totalSteps;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSteps;

    emit stepProcessed(error);
    return result;
}

/** @brief 批量自适应滤波 @param inputs 输入序列 @param desireds 期望序列 @return 步骤结果列表 */
QList<AdaptiveFilter2::StepResult> AdaptiveFilter2::processBatch(
    const QVector<double>& inputs,
    const QVector<double>& desireds)
{
    QList<StepResult> results;
    int count = qMin(inputs.size(), desireds.size());
    results.reserve(count);

    for (int i = 0; i < count; ++i) {
        results.append(processStep(inputs[i], desireds[i]));
    }
    return results;
}

/** @brief 仅滤波不更新权重 @param input 输入信号 @return 滤波输出 */
double AdaptiveFilter2::filterOnly(double input) const
{
    double output = 0.0;
    /* 使用当前缓冲区状态进行前向计算 */
    for (int i = 0; i < m_order; ++i) {
        int idx = (m_bufferIndex - 1 - i + m_order) % m_order;
        output += m_weights[i] * m_inputBuffer[idx];
    }
    /* 加上新输入 */
    output += m_weights[0] * input;
    return output;
}

/* ── 状态查询 ── */

/** @brief 获取收敛信息 */
AdaptiveFilter2::ConvergenceInfo AdaptiveFilter2::convergenceInfo() const
{
    return m_convergence;
}

/** @brief 获取当前权重 */
QVector<double> AdaptiveFilter2::weights() const
{
    return m_weights;
}

/** @brief 重置滤波器 */
void AdaptiveFilter2::reset()
{
    m_weights.assign(m_order, 0.0);
    m_inputBuffer.assign(m_order, 0.0);
    m_bufferIndex = 0;
    m_convergence = ConvergenceInfo{};
    m_errorSum = 0.0;
    m_errorCount = 0;
    m_minSteadyError = 1e300;
}

/* ── 私有: 权重更新 ── */

/** @brief LMS权重更新: w = w + mu * e * x */
void AdaptiveFilter2::updateLMS(double error)
{
    for (int i = 0; i < m_order; ++i) {
        int idx = (m_bufferIndex - i + m_order) % m_order;
        m_weights[i] += m_mu * error * m_inputBuffer[idx];
    }
}

/** @brief NLMS权重更新: w = w + (mu / (eps + ||x||^2)) * e * x */
void AdaptiveFilter2::updateNLMS(double error)
{
    /* 计算输入功率 */
    double power = m_epsilon;
    for (int i = 0; i < m_order; ++i) {
        power += m_inputBuffer[i] * m_inputBuffer[i];
    }

    double normMu = m_mu / power;
    for (int i = 0; i < m_order; ++i) {
        int idx = (m_bufferIndex - i + m_order) % m_order;
        m_weights[i] += normMu * error * m_inputBuffer[idx];
    }
}

/** @brief SignLMS权重更新: w = w + mu * sign(e) * x */
void AdaptiveFilter2::updateSignLMS(double error)
{
    double signError = (error > 0.0) ? 1.0 : ((error < 0.0) ? -1.0 : 0.0);
    for (int i = 0; i < m_order; ++i) {
        int idx = (m_bufferIndex - i + m_order) % m_order;
        m_weights[i] += m_mu * signError * m_inputBuffer[idx];
    }
}

/* ── 私有: 收敛检测 ── */

/** @brief 检查是否已收敛 */
void AdaptiveFilter2::checkConvergence()
{
    if (m_errorCount < m_order * 2) return;

    double mse = m_errorSum / static_cast<double>(m_errorCount);

    /* 更新收敛信息 */
    m_convergence.steadyStateError = mse;

    if (mse < m_convThreshold && !m_convergence.isConverged) {
        m_convergence.isConverged = true;
        m_convergence.convergenceIter = m_stats.totalSteps;
        m_convergence.convergenceRate = 1.0 / static_cast<double>(m_stats.totalSteps);

        /* 计算失调量 */
        if (m_minSteadyError > 0 && m_minSteadyError < 1e300) {
            m_convergence.misadjustment = (mse - m_minSteadyError) / m_minSteadyError;
        } else {
            m_convergence.misadjustment = 0.0;
        }

        ++m_stats.totalConvergences;
        emit converged(m_convergence.convergenceIter, mse);
    }

    if (mse < m_minSteadyError) {
        m_minSteadyError = mse;
    }

    /* 滑动窗口重置(避免历史累积) */
    if (m_errorCount > m_order * 10) {
        m_errorSum *= 0.5;
        m_errorCount /= 2;
    }
}

/* ── 统计 ── */

/** @brief 重置统计 */
void AdaptiveFilter2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
