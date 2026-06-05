/**
 * @file BezierSpline.h
 * @brief Bezier样条曲线 — 控制点插值与评估
 *
 * 功能: 实现二次/三次Bezier曲线，支持de Casteljau算法、
 *       曲线细分、弧长近似，统计评估次数/控制点数/耗时。
 */
#ifndef BEZIERSPLINE_H
#define BEZIERSPLINE_H

#include <QObject>
#include <QVector>
#include <QPointF>

class BezierSpline : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalEvaluations = 0;
        quint64 totalCurvesCreated = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit BezierSpline(QObject* parent = nullptr);

    /** @brief 设置三次Bezier控制点 @param p0~p3 四个控制点 */
    void setCubic(const QPointF& p0, const QPointF& p1,
                  const QPointF& p2, const QPointF& p3);

    /** @brief 设置二次Bezier控制点 @param p0~p2 三个控制点 */
    void setQuadratic(const QPointF& p0, const QPointF& p1,
                      const QPointF& p2);

    /** @brief 评估曲线 @param t 参数[0,1] @return 曲线上的点 */
    QPointF evaluate(double t) const;

    /** @brief 批量评估 @param numPoints 采样点数 @return 曲线采样点 */
    QVector<QPointF> evaluateRange(int numPoints = 100);

    /** @brief de Casteljau细分 @param t 细分参数 @return {左子曲线,右子曲线}控制点 */
    QPair<QVector<QPointF>, QVector<QPointF>> subdivide(double t) const;

    /** @brief 弧长近似 @param segments 分段数 @return 弧长 */
    double arcLength(int segments = 100) const;

    /** @brief 曲率 @param t 参数 @return 曲率值 */
    double curvature(double t) const;

    int degree() const { return m_degree; }
    const QVector<QPointF>& controlPoints() const { return m_points; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void curveEvaluated(int numPoints);

private:
    QPointF deCasteljau(const QVector<QPointF>& points, double t) const;

    QVector<QPointF> m_points;
    int m_degree;
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // BEZIERSPLINE_H
