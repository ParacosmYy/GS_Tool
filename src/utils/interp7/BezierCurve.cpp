/**
 * @file BezierCurve.cpp
 * @brief Bezier曲线求值实现 — De Casteljau递推算法
 *
 * De Casteljau算法通过逐层线性插值稳定地计算Bezier曲线上的点，
 * 同时保留中间层用于切线计算和曲线分割。
 */

#include "utils/interp7/BezierCurve.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ── 构造函数 ──

/** @brief 构造函数 @param parent 父对象 */
BezierCurve::BezierCurve(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BezierCurve"));
}

// ── 控制点设置 ──

/**
 * @brief 设置控制点
 * @param points 控制点列表(至少2个点定义线性Bezier曲线)
 */
void BezierCurve::setControlPoints(const QVector<QPointF>& points)
{
    m_controlPoints = points;
}

/** @brief 获取控制点 */
QVector<QPointF> BezierCurve::controlPoints() const
{
    return m_controlPoints;
}

/** @brief 获取曲线阶数 */
int BezierCurve::degree() const
{
    return qMax(0, m_controlPoints.size() - 1);
}

// ── 求值 ──

/**
 * @brief 使用De Casteljau算法求值
 * @param t 参数值[0,1]
 * @return 曲线上的点
 *
 * De Casteljau: 第k层第i个点 = lerp(P_{k-1,i}, P_{k-1,i+1}, t)
 * 最终只剩一个点即为曲线值。
 */
QPointF BezierCurve::evaluate(double t) const
{
    if (m_controlPoints.isEmpty()) return {};
    if (m_controlPoints.size() == 1) return m_controlPoints[0];

    t = qBound(0.0, t, 1.0);
    return deCasteljau(m_controlPoints, t);
}

/**
 * @brief 求值并计算导数信息
 * @param t 参数值[0,1]
 * @return 曲线点(含切线、法线、曲率)
 *
 * 切线 = De Casteljau倒数第二层两个点的差向量 * degree
 * 曲率 = |x'y'' - y'x''| / (x'^2 + y'^2)^(3/2)
 */
BezierCurve::CurvePoint BezierCurve::evaluateWithDerivatives(
    double t) const
{
    CurvePoint result;
    if (m_controlPoints.size() < 2) {
        result.position = m_controlPoints.isEmpty()
            ? QPointF() : m_controlPoints[0];
        return result;
    }

    t = qBound(0.0, t, 1.0);
    int n = degree();

    /* 获取De Casteljau各层 */
    auto layers = deCasteljauLayers(m_controlPoints, t);
    int lastLayer = layers.size() - 1;
    result.position = layers[lastLayer][0];

    /* 切线: 倒数第二层差向量 * n */
    if (layers.size() >= 2) {
        QPointF p0 = layers[lastLayer - 1][0];
        QPointF p1 = layers[lastLayer - 1][1];
        result.tangent = (p1 - p0) * static_cast<double>(n);
    }

    /* 法线: 切线旋转90度并单位化 */
    double tLen = qSqrt(result.tangent.x() * result.tangent.x()
        + result.tangent.y() * result.tangent.y());
    if (tLen > 1e-12) {
        result.normal = QPointF(-result.tangent.y() / tLen,
            result.tangent.x() / tLen);
    }

    /* 曲率 */
    QPointF d1 = tangent(t);
    QPointF d2 = secondDerivative(t);
    result.curvature = computeCurvature(d1, d2);

    return result;
}

/**
 * @brief 计算一阶导数(切线)
 * @param t 参数值[0,1]
 * @return 切线向量
 *
 * Bezier曲线导数: B'(t) = n * sum_{i=0}^{n-1} (P_{i+1} - P_i) * B_{n-1,i}(t)
 * 利用降阶控制点 + De Casteljau实现。
 */
QPointF BezierCurve::tangent(double t) const
{
    if (m_controlPoints.size() < 2) return {};
    t = qBound(0.0, t, 1.0);

    int n = degree();
    QVector<QPointF> diffPts;
    diffPts.reserve(n);
    for (int i = 0; i < n; ++i) {
        diffPts.append(m_controlPoints[i + 1] - m_controlPoints[i]);
    }

    QPointF d = deCasteljau(diffPts, t);
    return d * static_cast<double>(n);
}

/**
 * @brief 计算二阶导数
 * @param t 参数值[0,1]
 * @return 二阶导数向量
 */
QPointF BezierCurve::secondDerivative(double t) const
{
    if (m_controlPoints.size() < 3) return {};
    t = qBound(0.0, t, 1.0);

    int n = degree();

    /* 二阶差分控制点 */
    QVector<QPointF> diff2;
    diff2.reserve(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        diff2.append(m_controlPoints[i + 2] - 2.0 * m_controlPoints[i + 1]
            + m_controlPoints[i]);
    }

    QPointF d = deCasteljau(diff2, t);
    return d * static_cast<double>(n * (n - 1));
}

// ── 曲线分割 ──

/**
 * @brief 曲线分割(De Casteljau)
 * @param t 分割参数
 * @return 左右两条子曲线的控制点
 *
 * 利用De Casteljau的中间层: 左曲线取每层第一个点,
 * 右曲线取每层最后一个点。
 */
BezierCurve::SplitResult BezierCurve::split(double t) const
{
    SplitResult result;
    if (m_controlPoints.size() < 2) {
        result.left = m_controlPoints;
        result.right = m_controlPoints;
        return result;
    }

    t = qBound(0.0, t, 1.0);
    ++const_cast<BezierCurve*>(this)->m_stats.totalSplits;

    auto layers = deCasteljauLayers(m_controlPoints, t);

    /* 左曲线: 每层第一个点 */
    for (const auto& layer : layers) {
        result.left.append(layer[0]);
    }

    /* 右曲线: 每层最后一个点(逆序) */
    for (int i = layers.size() - 1; i >= 0; --i) {
        result.right.append(layers[i][layers[i].size() - 1]);
    }

    return result;
}

// ── 采样 ──

/**
 * @brief 均匀参数采样
 * @param numSamples 采样点数
 * @return 采样点列表
 */
QVector<QPointF> BezierCurve::sample(int numSamples) const
{
    ++const_cast<BezierCurve*>(this)->m_stats.totalSamplings;

    QVector<QPointF> points;
    points.reserve(numSamples);
    if (numSamples < 1 || m_controlPoints.size() < 2) return points;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i)
            / static_cast<double>(numSamples - 1);
        points.append(evaluate(t));
    }
    return points;
}

/**
 * @brief 采样含导数信息
 * @param numSamples 采样点数
 * @return 曲线点列表(含切线、法线、曲率)
 */
QVector<BezierCurve::CurvePoint> BezierCurve::sampleWithDerivatives(
    int numSamples) const
{
    ++const_cast<BezierCurve*>(this)->m_stats.totalSamplings;

    QVector<CurvePoint> points;
    points.reserve(numSamples);
    if (numSamples < 1 || m_controlPoints.size() < 2) return points;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i)
            / static_cast<double>(numSamples - 1);
        points.append(evaluateWithDerivatives(t));
    }
    return points;
}

// ── 弧长 ──

/**
 * @brief 计算近似弧长(Gauss-Legendre数值积分)
 * @param segments 积分段数
 * @return 弧长
 */
double BezierCurve::arcLength(int segments) const
{
    ++const_cast<BezierCurve*>(this)->m_stats.totalArcLengthComputations;

    if (m_controlPoints.size() < 2) return 0.0;

    double total = 0.0;
    double dt = 1.0 / static_cast<double>(segments);

    for (int i = 0; i < segments; ++i) {
        double t0 = static_cast<double>(i) * dt;
        double t1 = t0 + dt;
        double tMid = (t0 + t1) * 0.5;

        /* Simpson公式: h/6 * (f(a) + 4f(m) + f(b)) */
        QPointF d0 = tangent(t0);
        QPointF dM = tangent(tMid);
        QPointF d1 = tangent(t1);

        double s0 = qSqrt(d0.x() * d0.x() + d0.y() * d0.y());
        double sM = qSqrt(dM.x() * dM.x() + dM.y() * dM.y());
        double s1 = qSqrt(d1.x() * d1.x() + d1.y() * d1.y());

        total += dt / 6.0 * (s0 + 4.0 * sM + s1);
    }
    return total;
}

/**
 * @brief 提升阶数(增加一个控制点不改变曲线形状)
 * @return 提阶后的控制点(n+2个)
 *
 * 公式: Q_0 = P_0, Q_{n+1} = P_n
 * Q_i = i/(n+1) * P_{i-1} + (n+1-i)/(n+1) * P_i
 */
QVector<QPointF> BezierCurve::elevateDegree() const
{
    int n = degree();
    if (n < 1) return m_controlPoints;

    QVector<QPointF> newPts;
    newPts.reserve(n + 2);
    newPts.append(m_controlPoints[0]);

    for (int i = 1; i <= n; ++i) {
        double alpha = static_cast<double>(i) / static_cast<double>(n + 1);
        QPointF q = alpha * m_controlPoints[i - 1]
            + (1.0 - alpha) * m_controlPoints[i];
        newPts.append(q);
    }
    newPts.append(m_controlPoints[n]);
    return newPts;
}

/** @brief 重置统计 */
void BezierCurve::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ── De Casteljau核心 ──

/**
 * @brief De Casteljau递推核心
 * @param points 控制点
 * @param t 参数
 * @return 曲线上的点
 */
QPointF BezierCurve::deCasteljau(const QVector<QPointF>& points,
    double t) const
{
    QVector<QPointF> work = points;
    int n = work.size();
    double s = 1.0 - t;

    for (int k = 1; k < n; ++k) {
        for (int i = 0; i < n - k; ++i) {
            work[i] = s * work[i] + t * work[i + 1];
        }
    }
    return work[0];
}

/**
 * @brief De Casteljau递推保留中间层(用于分割和导数)
 * @param points 控制点
 * @param t 参数
 * @return 各层点集(0层=原始控制点, 最后一层=曲线点)
 */
QVector<QVector<QPointF>> BezierCurve::deCasteljauLayers(
    const QVector<QPointF>& points, double t) const
{
    QVector<QVector<QPointF>> layers;
    layers.append(points);

    double s = 1.0 - t;
    int n = points.size();

    for (int k = 1; k < n; ++k) {
        QVector<QPointF> layer;
        layer.reserve(n - k);
        const auto& prev = layers[k - 1];
        for (int i = 0; i < n - k; ++i) {
            layer.append(s * prev[i] + t * prev[i + 1]);
        }
        layers.append(layer);
    }
    return layers;
}

/** @brief 计算曲率 k = |x'y'' - y'x''| / (x'^2+y'^2)^{3/2} */
double BezierCurve::computeCurvature(const QPointF& d1,
    const QPointF& d2)
{
    double cross = cross2D(d1, d2);
    double dot = d1.x() * d1.x() + d1.y() * d1.y();
    if (dot < 1e-18) return 0.0;
    return qAbs(cross) / qPow(dot, 1.5);
}

/** @brief 二维叉积(a.x*b.y - a.y*b.x) */
double BezierCurve::cross2D(const QPointF& a, const QPointF& b)
{
    return a.x() * b.y() - a.y() * b.x();
}
