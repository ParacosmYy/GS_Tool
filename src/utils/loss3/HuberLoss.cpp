/**
 * @file HuberLoss.cpp
 * @brief Huber损失函数实现 — 鲁棒优化的平滑L1/L2混合损失
 */

#include "HuberLoss.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/* ---------- 构造函数 ---------- */

HuberLoss::HuberLoss(double delta, QObject* parent)
    : QObject(parent)
    , m_delta(delta)
{
}

/* ---------- 单样本计算 ---------- */

HuberLoss::LossResult HuberLoss::compute(double residual) const
{
    LossResult result;
    double absR = std::abs(residual);

    if (absR <= m_delta) {
        /* L2区域: loss = 0.5 * r^2, grad = r, hess = 1 */
        result.loss = 0.5 * residual * residual;
        result.gradient = residual;
        result.hessian = 1.0;
        result.isOutlier = false;
    } else {
        /* L1区域: loss = delta * (|r| - 0.5*delta), grad = delta*sign(r), hess = 0 */
        result.loss = m_delta * (absR - 0.5 * m_delta);
        result.gradient = m_delta * (residual > 0 ? 1.0 : -1.0);
        result.hessian = 0.0;
        result.isOutlier = true;
    }

    return result;
}

/* ---------- 批量计算 ---------- */

HuberLoss::BatchResult HuberLoss::computeBatch(
    const QVector<double>& residuals) const
{
    QElapsedTimer timer;
    timer.start();

    BatchResult result;
    int n = residuals.size();

    if (n == 0) {
        m_stats.totalComputed++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
        return result;
    }

    result.perSampleLoss.reserve(n);
    result.perSampleGrad.reserve(n);
    double totalLoss = 0.0;
    double totalGrad = 0.0;
    int outlierCount = 0;

    for (int i = 0; i < n; ++i) {
        LossResult lr = compute(residuals[i]);
        result.perSampleLoss.append(lr.loss);
        result.perSampleGrad.append(lr.gradient);
        totalLoss += lr.loss;
        totalGrad += lr.gradient;
        if (lr.isOutlier) outlierCount++;
    }

    result.totalLoss = totalLoss;
    result.avgLoss = totalLoss / n;
    result.totalGradient = totalGrad;
    result.outlierCount = outlierCount;
    result.inlierCount = n - outlierCount;

    m_stats.totalComputed++;
    m_stats.totalSamples += n;
    m_stats.totalOutliers += outlierCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit batchComputed(n, result.avgLoss, outlierCount);
    return result;
}

/* ---------- 仅损失值 ---------- */

double HuberLoss::loss(double residual) const
{
    double absR = std::abs(residual);
    if (absR <= m_delta) {
        return 0.5 * residual * residual;
    }
    return m_delta * (absR - 0.5 * m_delta);
}

/* ---------- 梯度 ---------- */

double HuberLoss::gradient(double residual) const
{
    double absR = std::abs(residual);
    if (absR <= m_delta) {
        return residual;
    }
    return m_delta * (residual > 0 ? 1.0 : -1.0);
}

/* ---------- 二阶导数 ---------- */

double HuberLoss::hessian(double residual) const
{
    double absR = std::abs(residual);
    return (absR <= m_delta) ? 1.0 : 0.0;
}

/* ---------- 自适应delta估计 ---------- */

double HuberLoss::estimateDelta(const QVector<double>& residuals) const
{
    if (residuals.isEmpty()) return m_delta;

    double mad = computeMAD(residuals);
    /* 1.4826是正态分布下MAD到标准差的转换系数 */
    /* delta = 1.345 * sigma 是Huber推荐值(95%高斯效率) */
    double sigma = 1.4826 * mad;
    return qMax(1.345 * sigma, 1e-6);
}

/* ---------- 从预测值计算 ---------- */

HuberLoss::LossResult HuberLoss::computeFromPrediction(
    double prediction, double target) const
{
    return compute(prediction - target);
}

/* ---------- 带权Huber损失 ---------- */

HuberLoss::BatchResult HuberLoss::computeWeighted(
    const QVector<double>& residuals,
    const QVector<double>& weights) const
{
    QElapsedTimer timer;
    timer.start();

    BatchResult result;
    int n = qMin(residuals.size(), weights.size());

    if (n == 0) {
        m_stats.totalComputed++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
        return result;
    }

    result.perSampleLoss.reserve(n);
    result.perSampleGrad.reserve(n);
    double totalLoss = 0.0;
    double totalGrad = 0.0;
    int outlierCount = 0;

    for (int i = 0; i < n; ++i) {
        LossResult lr = compute(residuals[i]);
        double w = weights[i];
        double weightedLoss = w * lr.loss;
        double weightedGrad = w * lr.gradient;

        result.perSampleLoss.append(weightedLoss);
        result.perSampleGrad.append(weightedGrad);
        totalLoss += weightedLoss;
        totalGrad += weightedGrad;
        if (lr.isOutlier) outlierCount++;
    }

    result.totalLoss = totalLoss;
    result.avgLoss = totalLoss / n;
    result.totalGradient = totalGrad;
    result.outlierCount = outlierCount;
    result.inlierCount = n - outlierCount;

    m_stats.totalComputed++;
    m_stats.totalSamples += n;
    m_stats.totalOutliers += outlierCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit batchComputed(n, result.avgLoss, outlierCount);
    return result;
}

/* ---------- 设置/获取delta ---------- */

void HuberLoss::setDelta(double delta)
{
    m_delta = qMax(delta, 1e-6);
}

double HuberLoss::delta() const { return m_delta; }

/* ---------- 私有: 中位数 ---------- */

double HuberLoss::median(QVector<double> values) const
{
    if (values.isEmpty()) return 0.0;
    std::sort(values.begin(), values.end());
    int n = values.size();
    if (n % 2 == 1) return values[n / 2];
    return (values[n / 2 - 1] + values[n / 2]) / 2.0;
}

/* ---------- 私有: MAD ---------- */

double HuberLoss::computeMAD(const QVector<double>& residuals) const
{
    if (residuals.isEmpty()) return 0.0;

    /* 先计算残差的中位数 */
    QVector<double> absRes;
    absRes.reserve(residuals.size());
    for (double r : residuals) {
        absRes.append(std::abs(r));
    }
    double med = median(absRes);

    /* 再计算 |r - median| 的中位数 */
    QVector<double> deviations;
    deviations.reserve(absRes.size());
    for (double ar : absRes) {
        deviations.append(std::abs(ar - med));
    }

    return median(deviations);
}

/* ---------- 统计 ---------- */

HuberLoss::Stats HuberLoss::stats() const { return m_stats; }

void HuberLoss::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
