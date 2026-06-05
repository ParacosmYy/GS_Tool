/**
 * @file BSplineInterp.cpp
 * @brief B样条插值器实现 — 节点插入与求值
 */

#include "utils/interp8/BSplineInterp.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

BSplineInterp::BSplineInterp(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

bool BSplineInterp::buildCurve(const QVector<Point2D>& controlPoints, int degree,
                                 KnotType knotType)
{
    QElapsedTimer timer;
    timer.start();

    if (controlPoints.size() < degree + 1) return false;

    m_controlPoints = controlPoints;
    m_degree = degree;

    int n = controlPoints.size();

    switch (knotType) {
    case KnotType::Clamped:
        m_knots = generateClampedKnots(n, degree);
        break;
    case KnotType::Uniform:
        m_knots = generateUniformKnots(n, degree);
        break;
    case KnotType::NonUniform:
        /* 非均匀节点必须由用户设置, 此处用钳位代替 */
        m_knots = generateClampedKnots(n, degree);
        break;
    }

    ++m_stats.totalCurvesBuilt;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalCurvesBuilt > 0)
        ? m_timeSum / m_stats.totalCurvesBuilt : 0.0;

    return true;
}

BSplineInterp::Point2D BSplineInterp::evaluate(double t) const
{
    if (m_controlPoints.isEmpty() || m_knots.isEmpty()) return {0.0, 0.0};

    /* 参数钳位 */
    t = qBound(m_knots[m_degree],
               t,
               m_knots[m_knots.size() - m_degree - 1] - 1e-10);

    int span = findKnotSpan(t);
    double x = deBoor(m_degree, span, t, 0);
    double y = deBoor(m_degree, span, t, 1);

    ++m_stats.totalEvaluations;

    return {x, y};
}

BSplineInterp::CurveSample BSplineInterp::evaluateWithDerivatives(double t) const
{
    CurveSample sample;
    if (m_controlPoints.isEmpty()) return sample;

    t = qBound(m_knots[m_degree],
               t,
               m_knots[m_knots.size() - m_degree - 1] - 1e-10);

    int span = findKnotSpan(t);

    /* 利用数值微分计算导数 */
    double dt = 1e-5;
    double t0 = qMax(m_knots[m_degree], t - dt);
    double t1 = qMin(m_knots[m_knots.size() - m_degree - 1], t + dt);

    Point2D p0 = {deBoor(m_degree, span, t0, 0), deBoor(m_degree, span, t0, 1)};
    Point2D p1 = {deBoor(m_degree, span, t1, 0), deBoor(m_degree, span, t1, 1)};

    sample.parameter = t;
    sample.x = deBoor(m_degree, span, t, 0);
    sample.y = deBoor(m_degree, span, t, 1);
    sample.dx = (p1.first - p0.first) / (t1 - t0);
    sample.dy = (p1.second - p0.second) / (t1 - t0);

    /* 曲率 κ = |x'y'' - y'x''| / (x'^2 + y'^2)^(3/2) */
    double dxdt = sample.dx;
    double dydt = sample.dy;
    double speed = qSqrt(dxdt * dxdt + dydt * dydt);
    sample.curvature = (speed > 1e-10) ? qFabs(dxdt * dydt - dydt * dxdt) /
        (speed * speed * speed) : 0.0;

    ++m_stats.totalEvaluations;
    return sample;
}

QVector<BSplineInterp::CurveSample> BSplineInterp::sampleCurve(int numSamples) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<CurveSample> samples(numSamples);
    if (m_controlPoints.isEmpty()) return samples;

    double tMin = m_knots[m_degree];
    double tMax = m_knots[m_knots.size() - m_degree - 1];

    for (int i = 0; i < numSamples; ++i) {
        double t = tMin + (tMax - tMin) * i / (numSamples - 1);
        samples[i] = evaluateWithDerivatives(t);
    }

    m_stats.totalSamplesGenerated += numSamples;
    m_timeSum += timer.elapsed();
    return samples;
}

bool BSplineInterp::insertKnot(double t)
{
    QElapsedTimer timer;
    timer.start();

    if (m_controlPoints.isEmpty() || m_knots.isEmpty()) return false;

    int span = findKnotSpan(t);
    int p = m_degree;
    int n = m_controlPoints.size();

    /* Boehm节点插入: 插入t到节点向量, 更新控制点 */
    QVector<Point2D> newPoints(n + 1);

    for (int i = 0; i <= span - p; ++i) {
        newPoints[i] = m_controlPoints[i];
    }

    for (int i = span - p + 1; i <= span; ++i) {
        double alpha = (t - m_knots[i]) / (m_knots[i + p] - m_knots[i]);
        double x = (1.0 - alpha) * m_controlPoints[i - 1].first
                   + alpha * m_controlPoints[i].first;
        double y = (1.0 - alpha) * m_controlPoints[i - 1].second
                   + alpha * m_controlPoints[i].second;
        newPoints[i] = {x, y};
    }

    for (int i = span + 1; i <= n; ++i) {
        newPoints[i] = m_controlPoints[i - 1];
    }

    /* 插入节点到节点向量 */
    m_knots.insert(span + 1, t);
    m_controlPoints = newPoints;

    ++m_stats.totalKnotInsertions;
    m_timeSum += timer.elapsed();
    return true;
}

QVector<BSplineInterp::Point2D> BSplineInterp::refineCurve(int subdivisions) const
{
    if (m_controlPoints.isEmpty()) return {};

    int p = m_degree;
    double tMin = m_knots[p];
    double tMax = m_knots[m_knots.size() - p - 1];

    QVector<Point2D> refined;
    int steps = (m_controlPoints.size() - 1) * subdivisions;
    for (int i = 0; i <= steps; ++i) {
        double t = tMin + (tMax - tMin) * i / steps;
        refined.append(evaluate(t));
    }

    return refined;
}

double BSplineInterp::arcLength(int numSegments) const
{
    if (m_controlPoints.isEmpty()) return 0.0;

    double tMin = m_knots[m_degree];
    double tMax = m_knots[m_knots.size() - m_degree - 1];
    double length = 0.0;

    Point2D prev = evaluate(tMin);
    for (int i = 1; i <= numSegments; ++i) {
        double t = tMin + (tMax - tMin) * i / numSegments;
        Point2D curr = evaluate(t);
        double dx = curr.first - prev.first;
        double dy = curr.second - prev.second;
        length += qSqrt(dx * dx + dy * dy);
        prev = curr;
    }

    return length;
}

BSplineInterp::Point2D BSplineInterp::evaluateByArcLength(double s) const
{
    if (s <= 0.0) return evaluate(m_knots[m_degree]);
    if (s >= 1.0) return evaluate(m_knots[m_knots.size() - m_degree - 1]);

    /* 二分搜索找参数t使弧长比例 ≈ s */
    double tMin = m_knots[m_degree];
    double tMax = m_knots[m_knots.size() - m_degree - 1];
    double totalLength = arcLength(200);
    double targetLength = s * totalLength;

    double lo = tMin, hi = tMax;
    for (int iter = 0; iter < 50; ++iter) {
        double mid = (lo + hi) / 2.0;
        /* 计算tMin到mid的弧长 */
        double partialLen = 0.0;
        int segs = 50;
        Point2D prev = evaluate(tMin);
        for (int i = 1; i <= segs; ++i) {
            double t = tMin + (mid - tMin) * i / segs;
            Point2D curr = evaluate(t);
            double dx = curr.first - prev.first;
            double dy = curr.second - prev.second;
            partialLen += qSqrt(dx * dx + dy * dy);
            prev = curr;
        }
        if (partialLen < targetLength) lo = mid;
        else hi = mid;
    }

    return evaluate((lo + hi) / 2.0);
}

QVector<BSplineInterp::Point2D> BSplineInterp::controlPoints() const
{
    return m_controlPoints;
}

QVector<double> BSplineInterp::knotVector() const
{
    return m_knots;
}

int BSplineInterp::degree() const
{
    return m_degree;
}

double BSplineInterp::deBoor(int deg, int knotSpan, double t, int component) const
{
    /* de Boor算法: 在knotSpan处递归求值 */
    QVector<double> d(deg + 1);
    for (int j = 0; j <= deg; ++j) {
        int idx = knotSpan - deg + j;
        if (idx >= 0 && idx < m_controlPoints.size()) {
            d[j] = (component == 0) ? m_controlPoints[idx].first
                                     : m_controlPoints[idx].second;
        }
    }

    for (int r = 1; r <= deg; ++r) {
        for (int j = deg; j >= r; --j) {
            int i = knotSpan - deg + j;
            double knotLeft = m_knots[i];
            double knotRight = m_knots[i + deg - r + 1];
            double denom = knotRight - knotLeft;
            if (qFabs(denom) < 1e-15) {
                d[j] = d[j - 1];
            } else {
                double alpha = (t - knotLeft) / denom;
                d[j] = (1.0 - alpha) * d[j - 1] + alpha * d[j];
            }
        }
    }

    return d[deg];
}

int BSplineInterp::findKnotSpan(double t) const
{
    int n = m_controlPoints.size() - 1;
    int p = m_degree;

    if (t >= m_knots[n + 1]) return n;
    if (t <= m_knots[p]) return p;

    /* 二分搜索 */
    int lo = p, hi = n + 1;
    int mid = (lo + hi) / 2;
    while (t < m_knots[mid] || t >= m_knots[mid + 1]) {
        if (t < m_knots[mid]) hi = mid;
        else lo = mid;
        mid = (lo + hi) / 2;
    }

    return mid;
}

QVector<double> BSplineInterp::generateClampedKnots(int n, int degree) const
{
    int m = n + degree + 1;
    QVector<double> knots(m);

    /* 前 degree+1 个节点为0, 后 degree+1 个节点为1 */
    for (int i = 0; i <= degree; ++i) knots[i] = 0.0;
    for (int i = m - degree - 1; i < m; ++i) knots[i] = 1.0;

    /* 中间均匀分布 */
    int innerCount = m - 2 * (degree + 1);
    for (int i = 0; i < innerCount; ++i) {
        knots[degree + 1 + i] = static_cast<double>(i + 1) / (innerCount + 1);
    }

    return knots;
}

QVector<double> BSplineInterp::generateUniformKnots(int n, int degree) const
{
    int m = n + degree + 1;
    QVector<double> knots(m);
    for (int i = 0; i < m; ++i) {
        knots[i] = static_cast<double>(i) / (m - 1);
    }
    return knots;
}

double BSplineInterp::basisFunction(int i, int p, double t) const
{
    /* Cox-de Boor递推公式 */
    if (p == 0) {
        return (t >= m_knots[i] && t < m_knots[i + 1]) ? 1.0 : 0.0;
    }

    double left = 0.0, right = 0.0;
    double denomLeft = m_knots[i + p] - m_knots[i];
    double denomRight = m_knots[i + p + 1] - m_knots[i + 1];

    if (qFabs(denomLeft) > 1e-15)
        left = ((t - m_knots[i]) / denomLeft) * basisFunction(i, p - 1, t);
    if (qFabs(denomRight) > 1e-15)
        right = ((m_knots[i + p + 1] - t) / denomRight) * basisFunction(i + 1, p - 1, t);

    return left + right;
}

BSplineInterp::Stats BSplineInterp::stats() const
{
    return m_stats;
}

void BSplineInterp::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
