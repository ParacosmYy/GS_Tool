/**
 * @file RobustRegression.cpp
 * @brief 鲁棒回归实现 — RANSAC + Theil-Sen
 */

#include "RobustRegression.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

/* ---------- 构造函数 ---------- */

RobustRegression::RobustRegression(QObject* parent)
    : QObject(parent)
{
}

/* ---------- RANSAC线性回归 ---------- */

RobustRegression::RegressionResult RobustRegression::fitRANSAC(
    const QVector<double>& x,
    const QVector<double>& y,
    double threshold,
    int maxIterations,
    double confidence) const
{
    QElapsedTimer timer;
    timer.start();

    RegressionResult result;
    int n = qMin(x.size(), y.size());
    if (n < 2) {
        result.valid = false;
        return result;
    }

    /* 自动阈值: 基于中位数绝对偏差(MAD) */
    double thresh = (threshold > 0.0) ? threshold : estimateThreshold(x, y);

    /* 自动迭代次数: log(1-p) / log(1-w^2) */
    int iterations = maxIterations;
    if (iterations <= 0) {
        double w = 0.5; /* 假设50%内点率 */
        double denom = std::log(1.0 - std::pow(w, 2));
        iterations = (std::abs(denom) < 1e-12)
            ? 1000
            : static_cast<int>(std::log(1.0 - confidence) / denom);
        iterations = qBound(50, iterations, 10000);
    }

    int bestInlierCount = 0;
    double bestSlope = 0.0, bestIntercept = 0.0;
    QVector<int> bestInliers;

    for (int iter = 0; iter < iterations; ++iter) {
        /* 随机选取2个点拟合直线 */
        int i1 = QRandomGenerator::global()->bounded(n);
        int i2 = QRandomGenerator::global()->bounded(n);
        while (i2 == i1) i2 = QRandomGenerator::global()->bounded(n);

        auto lineParams = fitLine(x[i1], y[i1], x[i2], y[i2]);
        double slope = lineParams.first;
        double intercept = lineParams.second;

        /* 统计内点 */
        QVector<int> inliers;
        for (int i = 0; i < n; ++i) {
            double res = std::abs(residual(x[i], y[i], slope, intercept));
            if (res <= thresh) {
                inliers.append(i);
            }
        }

        if (inliers.size() > bestInlierCount) {
            bestInlierCount = inliers.size();
            bestSlope = slope;
            bestIntercept = intercept;
            bestInliers = inliers;
        }

        /* 早停: 内点率已达置信度要求 */
        if (bestInlierCount >= static_cast<int>(confidence * n)) break;
    }

    /* 用所有内点做最小二乘精化 */
    if (bestInlierCount >= 2) {
        double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
        for (int idx : bestInliers) {
            sumX += x[idx];
            sumY += y[idx];
            sumXY += x[idx] * y[idx];
            sumXX += x[idx] * x[idx];
        }
        double denom = bestInlierCount * sumXX - sumX * sumX;
        if (std::abs(denom) > 1e-12) {
            bestSlope = (bestInlierCount * sumXY - sumX * sumY) / denom;
            bestIntercept = (sumY - bestSlope * sumX) / bestInlierCount;
        }

        /* 重新分类内点/外点 */
        bestInliers.clear();
        for (int i = 0; i < n; ++i) {
            double res = std::abs(residual(x[i], y[i], bestSlope, bestIntercept));
            if (res <= thresh) bestInliers.append(i);
        }
        bestInlierCount = bestInliers.size();
    }

    /* 构造结果 */
    result.slope = bestSlope;
    result.intercept = bestIntercept;
    result.inlierCount = bestInlierCount;
    result.outlierCount = n - bestInlierCount;
    result.inlierIndices = bestInliers;
    result.inlierThreshold = thresh;
    result.rSquared = computeRSquared(x, y, bestSlope, bestIntercept);
    result.valid = true;

    /* 外点索引 */
    QSet<int> inlierSet(bestInliers.begin(), bestInliers.end());
    for (int i = 0; i < n; ++i) {
        if (!inlierSet.contains(i)) result.outlierIndices.append(i);
    }

    m_stats.totalFits++;
    m_stats.totalInliers += bestInlierCount;
    m_stats.totalOutliers += result.outlierCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(result.inlierCount, result.outlierCount, result.rSquared);
    return result;
}

/* ---------- Theil-Sen中位数回归 ---------- */

RobustRegression::RegressionResult RobustRegression::fitTheilSen(
    const QVector<double>& x,
    const QVector<double>& y) const
{
    QElapsedTimer timer;
    timer.start();

    RegressionResult result;
    int n = qMin(x.size(), y.size());
    if (n < 2) {
        result.valid = false;
        return result;
    }

    /* 计算所有点对的斜率 */
    QVector<double> slopes;
    slopes.reserve(n * (n - 1) / 2);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dx = x[j] - x[i];
            if (std::abs(dx) > 1e-12) {
                slopes.append((y[j] - y[i]) / dx);
            }
        }
    }

    if (slopes.isEmpty()) {
        result.valid = false;
        return result;
    }

    /* 中位数斜率 */
    double slope = median(slopes);

    /* 中位数截距: intercept = median(y_i - slope * x_i) */
    QVector<double> intercepts;
    intercepts.reserve(n);
    for (int i = 0; i < n; ++i) {
        intercepts.append(y[i] - slope * x[i]);
    }
    double intercept = median(intercepts);

    result.slope = slope;
    result.intercept = intercept;
    result.inlierCount = n;
    result.outlierCount = 0;
    result.rSquared = computeRSquared(x, y, slope, intercept);
    result.valid = true;

    /* 用MAD标记外点 */
    QVector<double> residuals;
    residuals.reserve(n);
    for (int i = 0; i < n; ++i) {
        residuals.append(std::abs(residual(x[i], y[i], slope, intercept)));
    }
    double medRes = median(residuals);
    double madThresh = medRes * 3.0; /* 3*MAD阈值 */

    for (int i = 0; i < n; ++i) {
        if (residuals[i] > madThresh) {
            result.outlierIndices.append(i);
            result.outlierCount++;
        } else {
            result.inlierIndices.append(i);
        }
    }
    result.inlierCount = n - result.outlierCount;

    m_stats.totalFits++;
    m_stats.totalInliers += result.inlierCount;
    m_stats.totalOutliers += result.outlierCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(result.inlierCount, result.outlierCount, result.rSquared);
    return result;
}

/* ---------- 组合鲁棒回归 ---------- */

RobustRegression::RegressionResult RobustRegression::fitRobust(
    const QVector<double>& x,
    const QVector<double>& y,
    double threshold) const
{
    QElapsedTimer timer;
    timer.start();

    /* Step 1: RANSAC粗拟合 */
    auto ransacResult = fitRANSAC(x, y, threshold);

    if (!ransacResult.valid || ransacResult.inlierIndices.size() < 3) {
        m_stats.totalFits++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;
        return ransacResult;
    }

    /* Step 2: 内点上做Theil-Sen精化 */
    QVector<double> inlierX, inlierY;
    for (int idx : ransacResult.inlierIndices) {
        inlierX.append(x[idx]);
        inlierY.append(y[idx]);
    }

    auto tsResult = fitTheilSen(inlierX, inlierY);

    /* Step 3: 合并结果 */
    RegressionResult result;
    result.slope = tsResult.slope;
    result.intercept = tsResult.intercept;
    result.rSquared = computeRSquared(x, y, tsResult.slope, tsResult.intercept);
    result.inlierThreshold = ransacResult.inlierThreshold;
    result.valid = true;

    /* 用精化后的参数重新分类 */
    double thresh = (threshold > 0) ? threshold : ransacResult.inlierThreshold;
    int n = qMin(x.size(), y.size());
    for (int i = 0; i < n; ++i) {
        double res = std::abs(residual(x[i], y[i], result.slope, result.intercept));
        if (res <= thresh * 1.5) {
            result.inlierIndices.append(i);
        } else {
            result.outlierIndices.append(i);
        }
    }
    result.inlierCount = result.inlierIndices.size();
    result.outlierCount = result.outlierIndices.size();

    m_stats.totalFits++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(result.inlierCount, result.outlierCount, result.rSquared);
    return result;
}

/* ---------- R^2计算 ---------- */

double RobustRegression::computeRSquared(const QVector<double>& x,
                                          const QVector<double>& y,
                                          double slope,
                                          double intercept) const
{
    int n = qMin(x.size(), y.size());
    if (n < 2) return 0.0;

    double meanY = 0.0;
    for (int i = 0; i < n; ++i) meanY += y[i];
    meanY /= n;

    double ssTot = 0.0, ssRes = 0.0;
    for (int i = 0; i < n; ++i) {
        double pred = slope * x[i] + intercept;
        ssRes += (y[i] - pred) * (y[i] - pred);
        ssTot += (y[i] - meanY) * (y[i] - meanY);
    }

    if (ssTot < 1e-12) return 1.0;
    return 1.0 - ssRes / ssTot;
}

/* ---------- 私有: 两点拟合直线 ---------- */

QPair<double, double> RobustRegression::fitLine(
    double x1, double y1, double x2, double y2) const
{
    double dx = x2 - x1;
    if (std::abs(dx) < 1e-12) {
        return {0.0, y1}; /* 垂直线退化 */
    }
    double slope = (y2 - y1) / dx;
    double intercept = y1 - slope * x1;
    return {slope, intercept};
}

/* ---------- 私有: 残差 ---------- */

double RobustRegression::residual(double x, double y,
                                    double slope, double intercept) const
{
    return y - (slope * x + intercept);
}

/* ---------- 私有: 中位数 ---------- */

double RobustRegression::median(QVector<double> values) const
{
    if (values.isEmpty()) return 0.0;
    std::sort(values.begin(), values.end());
    int n = values.size();
    if (n % 2 == 1) return values[n / 2];
    return (values[n / 2 - 1] + values[n / 2]) / 2.0;
}

/* ---------- 私有: MAD自动阈值 ---------- */

double RobustRegression::estimateThreshold(
    const QVector<double>& x, const QVector<double>& y) const
{
    int n = qMin(x.size(), y.size());
    if (n < 3) return 1.0;

    /* 初始OLS拟合 */
    double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
    for (int i = 0; i < n; ++i) {
        sumX += x[i]; sumY += y[i];
        sumXY += x[i] * y[i]; sumXX += x[i] * x[i];
    }
    double denom = n * sumXX - sumX * sumX;
    if (std::abs(denom) < 1e-12) return 1.0;

    double slope = (n * sumXY - sumX * sumY) / denom;
    double intercept = (sumY - slope * sumX) / n;

    /* 计算残差MAD */
    QVector<double> res;
    for (int i = 0; i < n; ++i) {
        res.append(std::abs(y[i] - (slope * x[i] + intercept)));
    }
    double medRes = median(res);
    /* 阈值 = 2.5 * MAD (约覆盖98.7%正态分布) */
    return qMax(2.5 * medRes, 1e-6);
}

/* ---------- 统计 ---------- */

RobustRegression::Stats RobustRegression::stats() const { return m_stats; }

void RobustRegression::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
