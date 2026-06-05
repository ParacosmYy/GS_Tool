/**
 * @file BezierSpline.cpp
 * @brief Bezier样条曲线实现
 */

#include "utils/bezier/BezierSpline.h"

#include <QElapsedTimer>
#include <QtMath>

BezierSpline::BezierSpline(QObject* parent)
    : QObject(parent), m_degree(0), m_timeSum(0.0) {}

void BezierSpline::setCubic(const QPointF& p0, const QPointF& p1,
                             const QPointF& p2, const QPointF& p3)
{
    m_points = {p0, p1, p2, p3};
    m_degree = 3;
    m_stats.totalCurvesCreated++;
}

void BezierSpline::setQuadratic(const QPointF& p0, const QPointF& p1,
                                 const QPointF& p2)
{
    m_points = {p0, p1, p2};
    m_degree = 2;
    m_stats.totalCurvesCreated++;
}

QPointF BezierSpline::evaluate(double t) const
{
    t = qBound(0.0, t, 1.0);
    return deCasteljau(m_points, t);
}

QVector<QPointF> BezierSpline::evaluateRange(int numPoints)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPointF> result;
    result.reserve(numPoints);
    for (int i = 0; i <= numPoints; ++i) {
        double t = static_cast<double>(i) / numPoints;
        result.append(deCasteljau(m_points, t));
    }

    m_stats.totalEvaluations += numPoints;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalEvaluations, 1ULL);

    emit curveEvaluated(numPoints);
    return result;
}

QPair<QVector<QPointF>, QVector<QPointF>> BezierSpline::subdivide(
    double t) const
{
    t = qBound(0.0, t, 1.0);
    int n = m_points.size();

    /* 构建de Casteljau三角 */
    QVector<QVector<QPointF>> table(n);
    table[0] = m_points;
    for (int r = 1; r < n; ++r) {
        table[r].resize(n - r);
        for (int i = 0; i < n - r; ++i) {
            table[r][i] = (1.0 - t) * table[r - 1][i] +
                           t * table[r - 1][i + 1];
        }
    }

    /* 左子: 三角左列, 右子: 三角右列 */
    QVector<QPointF> left(n), right(n);
    for (int i = 0; i < n; ++i) {
        left[i] = table[i][0];
        right[i] = table[n - 1 - i][i];
    }

    return {left, right};
}

double BezierSpline::arcLength(int segments) const
{
    double length = 0.0;
    QPointF prev = deCasteljau(m_points, 0.0);

    for (int i = 1; i <= segments; ++i) {
        double t = static_cast<double>(i) / segments;
        QPointF curr = deCasteljau(m_points, t);
        double dx = curr.x() - prev.x();
        double dy = curr.y() - prev.y();
        length += qSqrt(dx * dx + dy * dy);
        prev = curr;
    }

    return length;
}

double BezierSpline::curvature(double t) const
{
    t = qBound(0.0, t, 1.0);

    double dt = 1e-4;
    double t0 = qMax(0.0, t - dt);
    double t1 = qMin(1.0, t + dt);
    double t2 = qMin(1.0, t + 2 * dt);

    QPointF p0 = deCasteljau(m_points, t0);
    QPointF p1 = deCasteljau(m_points, t1);
    QPointF p2 = deCasteljau(m_points, t2);

    /* 一阶和二阶差分 */
    QPointF d1 = (p1 - p0) / (t1 - t0);
    QPointF d2 = (p2 - 2 * p1 + p0) / ((t1 - t0) * (t1 - t0));

    double cross = d1.x() * d2.y() - d1.y() * d2.x();
    double speed = qSqrt(d1.x() * d1.x() + d1.y() * d1.y());

    return (speed > 1e-12) ? cross / (speed * speed * speed) : 0.0;
}

QPointF BezierSpline::deCasteljau(const QVector<QPointF>& points,
                                   double t) const
{
    QVector<QPointF> work = points;
    int n = work.size();
    for (int r = 1; r < n; ++r) {
        for (int i = 0; i < n - r; ++i) {
            work[i] = (1.0 - t) * work[i] + t * work[i + 1];
        }
    }
    return work[0];
}

void BezierSpline::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
