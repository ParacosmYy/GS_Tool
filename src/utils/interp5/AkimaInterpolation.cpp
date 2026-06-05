/**
 * @file AkimaInterpolation.cpp
 * @brief Akima插值实现 — 平滑非振荡分段三次插值
 */

#include "utils/interp5/AkimaInterpolation.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
AkimaInterpolation::AkimaInterpolation(QObject* parent)
    : QObject(parent)
    , m_valid(false)
{
}

/** @brief 计算Akima斜率 @param x X @param y Y @return 斜率数组 */
QVector<double> AkimaInterpolation::computeSlopes(const QVector<double>& x,
                                                   const QVector<double>& y) const
{
    int n = x.size();
    QVector<double> slopes(n, 0.0);

    /* 计算相邻点斜率 s[i] = (y[i+1]-y[i])/(x[i+1]-x[i]) */
    QVector<double> s(n + 3, 0.0);
    for (int i = 0; i < n - 1; ++i) {
        double dx = x[i + 1] - x[i];
        s[i + 2] = (dx > 1e-15) ? (y[i + 1] - y[i]) / dx : 0.0;
    }

    /* 边界延伸: s[-2], s[-1], s[n-1], s[n] (使用线性外推) */
    s[1] = 2.0 * s[2] - s[3];
    s[0] = 2.0 * s[1] - s[2];
    s[n + 1] = 2.0 * s[n] - s[n - 1];
    s[n + 2] = 2.0 * s[n + 1] - s[n];

    /* Akima权重公式 */
    for (int i = 0; i < n; ++i) {
        int idx = i + 2; /* 对应s数组中的中心斜率 */
        double w1 = std::abs(s[idx + 1] - s[idx]);
        double w2 = std::abs(s[idx - 1] - s[idx - 2]);

        if (w1 + w2 < 1e-15) {
            /* 权重都为0: 取相邻斜率平均值 */
            slopes[i] = 0.5 * (s[idx - 1] + s[idx]);
        } else {
            slopes[i] = (w1 * s[idx - 1] + w2 * s[idx]) / (w1 + w2);
        }
    }

    return slopes;
}

/** @brief 设置数据点 @param x X坐标 @param y Y坐标 @return 是否有效 */
bool AkimaInterpolation::setPoints(const QVector<double>& x, const QVector<double>& y)
{
    if (x.size() != y.size() || x.size() < 2) {
        m_valid = false;
        return false;
    }

    /* 检查严格递增 */
    for (int i = 1; i < x.size(); ++i) {
        if (x[i] <= x[i - 1]) {
            m_valid = false;
            return false;
        }
    }

    m_x.assign(x.begin(), x.end());
    m_y.assign(y.begin(), y.end());

    QVector<double> xVec(x.begin(), x.end());
    QVector<double> yVec(y.begin(), y.end());
    QVector<double> sl = computeSlopes(xVec, yVec);
    m_slopes.assign(sl.begin(), sl.end());

    m_valid = true;
    m_stats.totalKnots += static_cast<quint64>(x.size());
    return true;
}

/** @brief 查找x所在区间 @param x 目标x @return 区间索引 */
int AkimaInterpolation::findSegment(double x) const
{
    int n = static_cast<int>(m_x.size());
    if (n < 2) return 0;

    /* 二分查找 */
    int lo = 0, hi = n - 2;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (x > m_x[mid + 1]) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}

/** @brief 单点插值 @param x 目标X @return 插值Y */
double AkimaInterpolation::interpolate(double x) const
{
    if (!m_valid || m_x.size() < 2) return 0.0;

    int n = static_cast<int>(m_x.size());

    /* 边界外: 线性外推 */
    if (x <= m_x[0]) {
        double dx = m_x[1] - m_x[0];
        if (dx < 1e-15) return m_y[0];
        return m_y[0] + m_slopes[0] * (x - m_x[0]);
    }
    if (x >= m_x[n - 1]) {
        double dx = m_x[n - 1] - m_x[n - 2];
        if (dx < 1e-15) return m_y[n - 1];
        return m_y[n - 1] + m_slopes[n - 1] * (x - m_x[n - 1]);
    }

    /* 找到区间 [x_i, x_{i+1}] */
    int i = findSegment(x);
    double x0 = m_x[i], x1 = m_x[i + 1];
    double y0 = m_y[i], y1 = m_y[i + 1];
    double t0 = m_slopes[i], t1 = m_slopes[i + 1];

    double dx = x1 - x0;
    if (dx < 1e-15) return y0;

    /* Hermite基函数 */
    double t = (x - x0) / dx;
    double h00 = (1.0 + 2.0 * t) * (1.0 - t) * (1.0 - t);
    double h10 = t * (1.0 - t) * (1.0 - t);
    double h01 = t * t * (3.0 - 2.0 * t);
    double h11 = t * t * (t - 1.0);

    return h00 * y0 + h10 * dx * t0 + h01 * y1 + h11 * dx * t1;
}

/** @brief 批量插值 @param xPoints 目标X数组 @return Y值数组 */
QVector<double> AkimaInterpolation::interpolateBatch(const QVector<double>& xPoints) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    result.reserve(xPoints.size());
    for (double xp : xPoints) {
        result.append(interpolate(xp));
    }

    m_stats.totalInterpolations += static_cast<quint64>(xPoints.size());
    m_stats.totalPointsComputed += static_cast<quint64>(xPoints.size());
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalInterpolations);

    emit interpolationCompleted(xPoints.size());
    return result;
}

/** @brief 均匀采样 @param numSamples 采样点数 @return (x, y) */
QPair<QVector<double>, QVector<double>> AkimaInterpolation::sample(int numSamples) const
{
    QPair<QVector<double>, QVector<double>> result;
    if (!m_valid || m_x.size() < 2 || numSamples < 2) return result;

    QElapsedTimer timer;
    timer.start();

    double xMin = m_x.front();
    double xMax = m_x.back();
    double step = (xMax - xMin) / (numSamples - 1);

    result.first.reserve(numSamples);
    result.second.reserve(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        double x = xMin + i * step;
        result.first.append(x);
        result.second.append(interpolate(x));
    }

    m_stats.totalInterpolations += static_cast<quint64>(numSamples);
    m_stats.totalPointsComputed += static_cast<quint64>(numSamples);
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalInterpolations);

    emit interpolationCompleted(numSamples);
    return result;
}

/** @brief 计算导数 @param x 目标X @return 导数值 */
double AkimaInterpolation::derivative(double x) const
{
    if (!m_valid || m_x.size() < 2) return 0.0;

    int n = static_cast<int>(m_x.size());

    if (x <= m_x[0]) return m_slopes[0];
    if (x >= m_x[n - 1]) return m_slopes[n - 1];

    int i = findSegment(x);
    double x0 = m_x[i], x1 = m_x[i + 1];
    double y0 = m_y[i], y1 = m_y[i + 1];
    double t0 = m_slopes[i], t1 = m_slopes[i + 1];

    double dx = x1 - x0;
    if (dx < 1e-15) return 0.0;

    double t = (x - x0) / dx;

    /* Hermite基函数的导数 */
    double dh00 = 6.0 * t * t - 6.0 * t;
    double dh10 = 3.0 * t * t - 4.0 * t + 1.0;
    double dh01 = -6.0 * t * t + 6.0 * t;
    double dh11 = 3.0 * t * t - 2.0 * t;

    return (dh00 * y0 + dh10 * dx * t0 + dh01 * y1 + dh11 * dx * t1) / dx;
}

/** @brief 重置统计 */
void AkimaInterpolation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
