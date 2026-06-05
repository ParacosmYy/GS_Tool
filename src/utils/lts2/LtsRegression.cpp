/**
 * @file LtsRegression.cpp
 * * @brief 最小截平方(LTS)稳健回归实现 — 随机子集搜索+集中迭代
 */

#include "utils/lts2/LtsRegression.h"

#include <QtMath>
#include <QRandomGenerator>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
LtsRegression::LtsRegression(QObject* parent)
    : QObject(parent)
    , m_slope(0.0)
    , m_intercept(0.0)
    , m_rSquared(0.0)
    , m_fitted(false)
    , m_timeSum(0.0)
{
}

/** @brief LTS拟合 @param x 自变量 @param y 因变量 @param trimRatio 截断比例 @return (斜率,截距) */
QPair<double, double> LtsRegression::fit(const QVector<double>& x,
                                          const QVector<double>& y,
                                          double trimRatio)
{
    QPair<double, double> result = {0.0, 0.0};
    if (x.size() != y.size() || x.size() < 3) return result;

    m_timer.start();

    int n = x.size();
    trimRatio = qBound(0.0, trimRatio, 0.5);
    int h = n - static_cast<int>(trimRatio * n); /* 保留样本数 */
    h = qMax(h, static_cast<int>(qCeil(n / 2.0)) + 1);
    h = qMin(h, n);

    double bestRSS = 1e30;
    double bestSlope = 0.0;
    double bestIntercept = 0.0;

    /* 阶段1: 随机子集搜索 — 从多个随机子集出发寻找好的初始解 */
    int numStarts = qMin(500, qMax(50, n * 2));
    for (int trial = 0; trial < numStarts; ++trial) {
        /* 随机抽取h个样本的索引 */
        QVector<int> indices(n);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(),
                     *QRandomGenerator::global());
        indices.resize(h);

        /* 对子集做OLS */
        QPair<double, double> ols = olsSubset(x, y, indices);

        /* 计算截断RSS */
        double rss = trimmedRSS(x, y, ols.first, ols.second, h);

        if (rss < bestRSS) {
            bestRSS = rss;
            bestSlope = ols.first;
            bestIntercept = ols.second;
        }
    }

    /* 阶段2: 集中迭代(C-step) — 反复用当前最优参数选h个最小残差样本 */
    for (int iter = 0; iter < 10; ++iter) {
        /* 计算所有残差并排序 */
        QVector<QPair<double, int>> residualIdx(n);
        for (int i = 0; i < n; ++i) {
            double pred = bestSlope * x[i] + bestIntercept;
            residualIdx[i] = {qAbs(y[i] - pred), i};
        }
        std::sort(residualIdx.begin(), residualIdx.end());

        /* 取残差最小的h个 */
        QVector<int> bestIdx(h);
        for (int i = 0; i < h; ++i) {
            bestIdx[i] = residualIdx[i].second;
        }

        /* 对新子集做OLS */
        QPair<double, double> ols = olsSubset(x, y, bestIdx);
        double rss = trimmedRSS(x, y, ols.first, ols.second, h);

        if (rss < bestRSS - 1e-12) {
            bestRSS = rss;
            bestSlope = ols.first;
            bestIntercept = ols.second;
        } else {
            break; /* 收敛 */
        }
    }

    m_slope = bestSlope;
    m_intercept = bestIntercept;
    m_fitted = true;

    /* 计算R²(使用保留样本) */
    QVector<QPair<double, int>> residualIdx(n);
    for (int i = 0; i < n; ++i) {
        double pred = m_slope * x[i] + m_intercept;
        residualIdx[i] = {(y[i] - pred) * (y[i] - pred), i};
    }
    std::sort(residualIdx.begin(), residualIdx.end());

    double yMean = 0.0;
    for (int i = 0; i < n; ++i) yMean += y[i];
    yMean /= n;

    double ssTot = 0.0;
    double ssRes = 0.0;
    for (int i = 0; i < h; ++i) {
        int idx = residualIdx[i].second;
        double diff = y[idx] - yMean;
        ssTot += diff * diff;
        ssRes += residualIdx[i].first;
    }
    m_rSquared = (ssTot > 1e-15) ? 1.0 - ssRes / ssTot : 0.0;
    m_rSquared = qBound(0.0, m_rSquared, 1.0);

    /* 统计更新 */
    ++m_stats.totalFits;
    double elapsed = m_timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFits);

    result = {m_slope, m_intercept};
    emit fitCompleted(m_slope, m_intercept, m_rSquared);
    return result;
}

/** @brief 计算残差 @param x 自变量 @param y 因变量 @return 残差向量 */
QVector<double> LtsRegression::residuals(const QVector<double>& x,
                                          const QVector<double>& y) const
{
    if (!m_fitted || x.size() != y.size()) return {};

    int n = x.size();
    QVector<double> res(n);
    for (int i = 0; i < n; ++i) {
        res[i] = y[i] - (m_slope * x[i] + m_intercept);
    }
    return res;
}

/** @brief 获取R² @return R²值 */
double LtsRegression::rSquared() const
{
    return m_rSquared;
}

/** @brief 重置统计 */
void LtsRegression::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 子集OLS拟合 @param x 自变量 @param y 因变量 @param indices 索引 @return (斜率,截距) */
QPair<double, double> LtsRegression::olsSubset(
    const QVector<double>& x, const QVector<double>& y,
    const QVector<int>& indices)
{
    int h = indices.size();
    if (h < 2) return {0.0, 0.0};

    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumXX = 0.0;
    for (int idx : indices) {
        sumX += x[idx];
        sumY += y[idx];
        sumXY += x[idx] * y[idx];
        sumXX += x[idx] * x[idx];
    }

    double denom = h * sumXX - sumX * sumX;
    if (qAbs(denom) < 1e-15) return {0.0, sumY / h};

    double slope = (h * sumXY - sumX * sumY) / denom;
    double intercept = (sumY - slope * sumX) / h;
    return {slope, intercept};
}

/** @brief 截断残差平方和 @param x 自变量 @param y 因变量 @param slope 斜率 @param intercept 截距 @param h 保留数 @return RSS */
double LtsRegression::trimmedRSS(const QVector<double>& x,
                                  const QVector<double>& y,
                                  double slope, double intercept, int h)
{
    int n = x.size();
    QVector<double> sqResiduals(n);
    for (int i = 0; i < n; ++i) {
        double r = y[i] - (slope * x[i] + intercept);
        sqResiduals[i] = r * r;
    }

    /* 部分排序: 只需找到前h小的 */
    std::nth_element(sqResiduals.begin(),
                     sqResiduals.begin() + h,
                     sqResiduals.end());

    double rss = 0.0;
    for (int i = 0; i < h; ++i) {
        rss += sqResiduals[i];
    }
    return rss;
}

/** @brief 完整数据OLS @param x 自变量 @param y 因变量 @return (斜率,截距) */
QPair<double, double> LtsRegression::olsFull(const QVector<double>& x,
                                              const QVector<double>& y)
{
    int n = x.size();
    if (n < 2) return {0.0, 0.0};

    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumXX = 0.0;
    for (int i = 0; i < n; ++i) {
        sumX += x[i];
        sumY += y[i];
        sumXY += x[i] * y[i];
        sumXX += x[i] * x[i];
    }

    double denom = n * sumXX - sumX * sumX;
    if (qAbs(denom) < 1e-15) return {0.0, sumY / n};

    double slope = (n * sumXY - sumX * sumY) / denom;
    double intercept = (sumY - slope * sumX) / n;
    return {slope, intercept};
}
