/**
 * @file DynamicTimeWarping.cpp
 * @brief 动态时间规整实现 — Sakoe-Chiba带约束
 */

#include "utils/distance3/DynamicTimeWarping.h"

#include <QtMath>
#include <QElapsedTimer>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
DynamicTimeWarping::DynamicTimeWarping(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置步进模式 @param pattern 步进模式 */
void DynamicTimeWarping::setStepPattern(StepPattern pattern)
{
    m_stepPattern = pattern;
}

/** @brief 设置距离度量 @param metric 距离度量 */
void DynamicTimeWarping::setDistanceMetric(DistanceMetric metric)
{
    m_metric = metric;
}

/** @brief 设置带宽约束 @param bandwidth 带宽 */
void DynamicTimeWarping::setBandWidth(int bandwidth)
{
    m_bandWidth = qMax(0, bandwidth);
}

/** @brief 计算DTW距离 @param series1 时间序列1 @param series2 时间序列2 @return DTW结果 */
DynamicTimeWarping::DtwResult DynamicTimeWarping::DynamicTimeWarping::compute(
    const QVector<double>& series1, const QVector<double>& series2)
{
    DtwResult result;

    if (series1.isEmpty() || series2.isEmpty()) {
        result.distance = std::numeric_limits<double>::infinity();
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    int n = series1.size();
    int m = series2.size();

    /* 带宽约束 */
    int w = m_bandWidth;
    if (w <= 0) w = qMax(n, m); /* 无约束 */
    w = qMax(w, qAbs(n - m));   /* 确保能到达终点 */

    /* 初始化代价矩阵 */
    QVector<QVector<double>> cost(n + 1, QVector<double>(m + 1,
        std::numeric_limits<double>::infinity()));
    cost[0][0] = 0.0;

    for (int i = 1; i <= n; ++i) {
        int jStart = qMax(1, i - w);
        int jEnd = qMin(m, i + w);

        for (int j = jStart; j <= jEnd; ++j) {
            double d = pointDistance(series1[i - 1], series2[j - 1]);

            double c1 = cost[i - 1][j];       /* 上方 */
            double c2 = cost[i][j - 1];       /* 左方 */
            double c3 = cost[i - 1][j - 1];   /* 对角 */

            switch (m_stepPattern) {
            case StepPattern::Symmetric:
                /* 对称: min(对角, 水平+垂直, 垂直+水平) */
                cost[i][j] = d + qMin(c3, qMin(c1, c2));
                break;
            case StepPattern::Asymmetric:
                /* 非对称: 只沿i轴步进 */
                cost[i][j] = d + qMin(c3, qMin(c1, cost[i - 1][j - 1]));
                break;
            case StepPattern::SymmetricP0:
                /* 对称P0: 简单对角/水平/垂直 */
                cost[i][j] = d + qMin(c3, qMin(c1, c2));
                break;
            }

            ++m_stats.totalCellsEvaluated;
        }
    }

    result.distance = cost[n][m];
    result.normalizedDistance = result.distance / (n + m);
    result.path = backtracePath(cost, n, m);

    /* 统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalComputations;
    ++m_stats.totalWarpsComputed;
    m_stats.totalDistanceSum += result.distance;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationComplete(result.distance, result.path.size());
    return result;
}

/** @brief 快速DTW @param series1 序列1 @param series2 序列2 @param radius 半径 @return DTW结果 */
DynamicTimeWarping::DtwResult DynamicTimeWarping::fastDtw(
    const QVector<double>& series1, const QVector<double>& series2, int radius)
{
    if (series1.isEmpty() || series2.isEmpty()) {
        return DtwResult{};
    }

    QElapsedTimer timer;
    timer.start();

    int minLen = 2 * radius + 1;
    if (series1.size() <= minLen && series2.size() <= minLen) {
        /* 基础情况: 直接计算 */
        return compute(series1, series2);
    }

    /* 降采样(2倍) */
    QVector<double> ds1 = downsample(series1, 2);
    QVector<double> ds2 = downsample(series2, 2);

    /* 递归求解粗DTW */
    DtwResult coarseResult = fastDtw(ds1, ds2, radius);

    /* 用粗路径扩展窗口 */
    QVector<QVector<bool>> window = expandWindow(
        coarseResult.path, radius, series1.size(), series2.size());

    /* 在窗口约束下重新计算 */
    int n = series1.size();
    int m = series2.size();

    QVector<QVector<double>> cost(n + 1, QVector<double>(m + 1,
        std::numeric_limits<double>::infinity()));
    cost[0][0] = 0.0;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (!window.isEmpty() && i - 1 < window.size()
                && j - 1 < window[i - 1].size() && !window[i - 1][j - 1]) {
                continue;
            }
            double d = pointDistance(series1[i - 1], series2[j - 1]);
            cost[i][j] = d + qMin(cost[i - 1][j - 1],
                                   qMin(cost[i - 1][j], cost[i][j - 1]));
        }
    }

    DtwResult result;
    result.distance = cost[n][m];
    result.normalizedDistance = result.distance / (n + m);
    result.path = backtracePath(cost, n, m);

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalComputations;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    return result;
}

/** @brief 距离矩阵 @param series 序列集合 @return 矩阵 */
QVector<QVector<double>> DynamicTimeWarping::distanceMatrix(
    const QVector<QVector<double>>& series)
{
    int n = series.size();
    QVector<QVector<double>> matrix(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            DtwResult r = compute(series[i], series[j]);
            matrix[i][j] = r.distance;
            matrix[j][i] = r.distance;
        }
    }
    return matrix;
}

/** @brief 重置统计 */
void DynamicTimeWarping::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算点距离 @param a 点A @param b 点B @return 距离 */
double DynamicTimeWarping::pointDistance(double a, double b) const
{
    switch (m_metric) {
    case DistanceMetric::Euclidean:
        return qAbs(a - b);
    case DistanceMetric::Manhattan:
        return qAbs(a - b);
    case DistanceMetric::Cosine: {
        double dot = a * b;
        double norm = qSqrt(a * a) * qSqrt(b * b);
        if (qFuzzyIsNull(norm)) return 1.0;
        return 1.0 - dot / norm;
    }
    }
    return qAbs(a - b);
}

/** @brief 回溯最优路径 @param cost 代价矩阵 @param n 行数 @param m 列数 @return 路径 */
QVector<QPair<int,int>> DynamicTimeWarping::backtracePath(
    const QVector<QVector<double>>& cost, int n, int m) const
{
    QVector<QPair<int,int>> path;
    int i = n, j = m;

    while (i > 0 && j > 0) {
        path.prepend({i - 1, j - 1});

        double c1 = (i > 0 && j > 0) ? cost[i - 1][j - 1]
            : std::numeric_limits<double>::infinity();
        double c2 = (i > 0) ? cost[i - 1][j]
            : std::numeric_limits<double>::infinity();
        double c3 = (j > 0) ? cost[i][j - 1]
            : std::numeric_limits<double>::infinity();

        double minCost = qMin(c1, qMin(c2, c3));
        if (minCost == c1) {
            --i; --j;
        } else if (minCost == c2) {
            --i;
        } else {
            --j;
        }
    }

    return path;
}

/** @brief 降采样 @param series 序列 @param factor 因子 @return 降采样序列 */
QVector<double> DynamicTimeWarping::downsample(
    const QVector<double>& series, int factor) const
{
    if (factor <= 1 || series.size() <= factor) return series;

    QVector<double> result;
    int outSize = (series.size() + factor - 1) / factor;
    result.reserve(outSize);

    for (int i = 0; i < series.size(); i += factor) {
        double sum = 0.0;
        int count = 0;
        for (int j = i; j < qMin(i + factor, series.size()); ++j) {
            sum += series[j];
            ++count;
        }
        result.append(sum / count);
    }
    return result;
}

/** @brief 扩展窗口 @param path 粗路径 @param radius 半径 @param n 序列1长度 @param m 序列2长度 */
QVector<QVector<bool>> DynamicTimeWarping::expandWindow(
    const QVector<QPair<int,int>>& path, int radius,
    int n, int m) const
{
    QVector<QVector<bool>> window(n, QVector<bool>(m, false));

    /* 将粗路径坐标放大到原始尺度 */
    for (const auto& p : path) {
        int ci = p.first * 2;
        int cj = p.second * 2;

        /* 扩展半径范围 */
        for (int di = -radius; di <= radius; ++di) {
            for (int dj = -radius; dj <= radius; ++dj) {
                int ni = ci + di;
                int nj = cj + dj;
                if (ni >= 0 && ni < n && nj >= 0 && nj < m) {
                    window[ni][nj] = true;
                }
            }
        }
    }
    return window;
}
