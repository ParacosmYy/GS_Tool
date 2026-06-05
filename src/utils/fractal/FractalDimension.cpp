/**
 * @file FractalDimension.cpp
 * @brief 分形维数估计器实现 — 盒计数/Higuchi
 */

#include "utils/fractal/FractalDimension.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
FractalDimension::FractalDimension(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

/** @brief 盒计数法 @param data 信号数据 @param minSize 最小盒子 @param maxSize 最大盒子 @return 分形维数 */
double FractalDimension::boxCountingDimension(
    const QVector<double>& data, int minSize, int maxSize)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 4) return 0.0;

    if (maxSize <= 0) maxSize = n / 2;
    if (minSize < 1) minSize = 1;

    /* 计算数据范围 */
    double minVal = *std::min_element(data.begin(), data.end());
    double maxVal = *std::max_element(data.begin(), data.end());
    double range = maxVal - minVal;
    if (range <= 0) return 0.0;

    /* 不同盒子尺寸下的计数 */
    QVector<QPair<double, double>> points;
    for (int size = minSize; size <= maxSize; size *= 2) {
        double epsilon = static_cast<double>(size);
        int count = 0;

        /* 时间轴方向: ceil(n/size)段 */
        int segments = (n + size - 1) / size;
        for (int seg = 0; seg < segments; ++seg) {
            double segMin = 1e30;
            double segMax = -1e30;
            int start = seg * size;
            int end = std::min(start + size, n);
            for (int i = start; i < end; ++i) {
                if (data[i] < segMin) segMin = data[i];
                if (data[i] > segMax) segMax = data[i];
            }
            /* 幅值方向需要的盒子数 */
            double segRange = segMax - segMin;
            double boxH = range / (range / epsilon);
            count += static_cast<int>(qCeil(segRange / boxH)) + 1;
        }

        if (count > 0) {
            points.append({qLn(epsilon), qLn(static_cast<double>(count))});
        }
    }

    /* 线性回归拟合斜率=分形维数 */
    double dim = 0.0;
    if (points.size() >= 2) {
        double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
        int m = points.size();
        for (int i = 0; i < m; ++i) {
            sumX += points[i].first;
            sumY += points[i].second;
            sumXY += points[i].first * points[i].second;
            sumX2 += points[i].first * points[i].first;
        }
        double denom = m * sumX2 - sumX * sumX;
        if (qAbs(denom) > 1e-12) {
            dim = (m * sumXY - sumX * sumY) / denom;
        }
    }

    m_stats.totalEstimations++;
    m_stats.lastBoxCountDim = dim;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalEstimations > 0) ? m_timeSum / m_stats.totalEstimations : 0.0;

    emit dimensionEstimated(dim, QStringLiteral("BoxCounting"));
    return dim;
}

/** @brief 盒计数曲线 @param data 信号数据 @return (log(size), log(count))点对 */
QVector<QPair<double, double>> FractalDimension::boxCountingCurve(
    const QVector<double>& data, int minSize, int maxSize)
{
    int n = data.size();
    if (n < 4) return {};

    if (maxSize <= 0) maxSize = n / 2;
    if (minSize < 1) minSize = 1;

    double minVal = *std::min_element(data.begin(), data.end());
    double maxVal = *std::max_element(data.begin(), data.end());
    double range = maxVal - minVal;
    if (range <= 0) return {};

    QVector<QPair<double, double>> points;
    for (int size = minSize; size <= maxSize; size *= 2) {
        double epsilon = static_cast<double>(size);
        int count = 0;
        int segments = (n + size - 1) / size;
        for (int seg = 0; seg < segments; ++seg) {
            double segMin = 1e30, segMax = -1e30;
            int start = seg * size;
            int end = std::min(start + size, n);
            for (int i = start; i < end; ++i) {
                if (data[i] < segMin) segMin = data[i];
                if (data[i] > segMax) segMax = data[i];
            }
            double segRange = segMax - segMin;
            double boxH = range / (range / epsilon);
            count += static_cast<int>(qCeil(segRange / boxH)) + 1;
        }
        if (count > 0) {
            points.append({qLn(epsilon), qLn(static_cast<double>(count))});
        }
    }
    return points;
}

/** @brief Higuchi分形维数 @param data 信号数据 @param maxK 最大k值 @return 分形维数 */
double FractalDimension::higuchiDimension(
    const QVector<double>& data, int maxK)
{
    QElapsedTimer timer;
    timer.start();

    int N = data.size();
    if (N < 10) return 0.0;

    if (maxK <= 0) maxK = N / 2;
    maxK = std::min(maxK, N / 2);
    if (maxK < 2) maxK = 2;

    QVector<QPair<double, double>> points;

    for (int k = 1; k <= maxK; ++k) {
        double Lmean = 0.0;
        int validM = 0;

        for (int m = 0; m < k; ++m) {
            /* 构建子序列长度 */
            int Nk = (N - m - 1) / k;
            if (Nk < 1) continue;

            double Lmk = 0.0;
            for (int i = 1; i <= Nk; ++i) {
                double diff = qAbs(data[m + i * k] - data[m + (i - 1) * k]);
                Lmk += diff;
            }

            /* 归一化 */
            double factor = static_cast<double>(N - 1) / (Nk * k);
            Lmk *= factor / k;

            Lmean += Lmk;
            validM++;
        }

        if (validM > 0) {
            Lmean /= validM;
            if (Lmean > 0) {
                points.append({qLn(1.0 / k), qLn(Lmean)});
            }
        }
    }

    /* 线性回归 */
    double dim = 0.0;
    if (points.size() >= 2) {
        double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
        int m = points.size();
        for (int i = 0; i < m; ++i) {
            sumX += points[i].first;
            sumY += points[i].second;
            sumXY += points[i].first * points[i].second;
            sumX2 += points[i].first * points[i].first;
        }
        double denom = m * sumX2 - sumX * sumX;
        if (qAbs(denom) > 1e-12) {
            dim = (m * sumXY - sumX * sumY) / denom;
        }
    }

    m_stats.totalEstimations++;
    m_stats.lastHiguchiDim = dim;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalEstimations > 0) ? m_timeSum / m_stats.totalEstimations : 0.0;

    emit dimensionEstimated(dim, QStringLiteral("Higuchi"));
    return dim;
}

/** @brief 信号复杂度评估 @param data 信号数据 @return 复杂度[0,1] */
double FractalDimension::complexityMeasure(const QVector<double>& data)
{
    double fd = boxCountingDimension(data);
    /* 分形维数范围约[1,2]，归一化到[0,1] */
    double complexity = (fd - 1.0);
    return std::max(0.0, std::min(1.0, complexity));
}

/** @brief 重置统计 */
void FractalDimension::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
