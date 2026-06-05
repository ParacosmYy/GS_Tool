/**
 * @file BezierCurve.h
 * @brief Bezier曲线求值 — De Casteljau递推算法
 *
 * 功能: 使用De Casteljau算法对任意阶Bezier曲线进行求值、
 *       切线计算、曲线分割、弧长估算、均匀采样，
 *       适用于传感器数据平滑、信号生成、动画插值等场景。
 *
 * 协作: SignalGenerator(信号生成) / DataSmoother(数据平滑)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Bezier曲线求值引擎
 *
 * 典型用法:
 * @code
 *   BezierCurve curve;
 *   curve.setControlPoints({{0,0}, {0.5,1}, {1,0}});
 *   QPointF pt = curve.evaluate(0.5);
 *   auto samples = curve.sample(50);
 * @endcode
 */
class BezierCurve : public QObject {
    Q_OBJECT

public:
    /** @brief 曲线评估结果 */
    struct CurvePoint {
        QPointF position;           ///< 曲线上的点
        QPointF tangent;            ///< 切线向量
        QPointF normal;             ///< 法线向量(单位化)
        double curvature = 0.0;     ///< 曲率
    };

    /** @brief 曲线分割结果 */
    struct SplitResult {
        QVector<QPointF> left;      ///< 左半控制点
        QVector<QPointF> right;     ///< 右半控制点
    };

    /** @brief 统计数据 */
    struct Stats {
        int totalEvaluations = 0;               ///< 累计求值次数
        int totalSamplings = 0;                 ///< 累计采样次数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理时间(ms)
        int totalSplits = 0;                    ///< 累计分割次数
        int totalArcLengthComputations = 0;     ///< 累计弧长计算次数
    };

    explicit BezierCurve(QObject* parent = nullptr);

    /**
     * @brief 设置控制点
     * @param points 控制点列表(至少2个)
     */
    void setControlPoints(const QVector<QPointF>& points);

    /** @brief 获取控制点 @return 控制点列表 */
    QVector<QPointF> controlPoints() const;

    /**
     * @brief 使用De Casteljau算法求值
     * @param t 参数值[0,1]
     * @return 曲线上的点
     */
    QPointF evaluate(double t) const;

    /**
     * @brief 求值并计算导数信息
     * @param t 参数值[0,1]
     * @return 曲线点(含切线/法线/曲率)
     */
    CurvePoint evaluateWithDerivatives(double t) const;

    /**
     * @brief 计算一阶导数(切线)
     * @param t 参数值[0,1]
     * @return 切线向量
     */
    QPointF tangent(double t) const;

    /**
     * @brief 计算二阶导数
     * @param t 参数值[0,1]
     * @return 二阶导数向量
     */
    QPointF secondDerivative(double t) const;

    /**
     * @brief 曲线分割(De Casteljau)
     * @param t 分割参数
     * @return 左右两条子曲线的控制点
     */
    SplitResult split(double t) const;

    /**
     * @brief 均匀采样曲线
     * @param numSamples 采样点数
     * @return 采样点列表
     */
    QVector<QPointF> sample(int numSamples) const;

    /**
     * @brief 采样含导数信息
     * @param numSamples 采样点数
     * @return 曲线点列表(含切线/法线/曲率)
     */
    QVector<CurvePoint> sampleWithDerivatives(int numSamples) const;

    /**
     * @brief 计算近似弧长(数值积分)
     * @param segments 积分段数
     * @return 弧长
     */
    double arcLength(int segments = 100) const;

    /**
     * @brief 提升阶数(增加一个控制点不改变曲线形状)
     * @return 提阶后的控制点
     */
    QVector<QPointF> elevateDegree() const;

    /** @brief 获取曲线阶数 @return 阶数(控制点数-1) */
    int degree() const;

    /** @brief 获取统计 @return 统计数据 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求值完成 @param t 参数值 @param point 曲线点 */
    void evaluated(double t, QPointF point);

    /** @brief 采样完成 @param count 采样点数 */
    void samplingCompleted(int count);

private:
    /** @brief De Casteljau递推核心 */
    QPointF deCasteljau(const QVector<QPointF>& points, double t) const;

    /** @brief De Casteljau递推(保留中间层用于分割) */
    QVector<QVector<QPointF>> deCasteljauLayers(
        const QVector<QPointF>& points, double t) const;

    /** @brief 计算曲率 */
    static double computeCurvature(const QPointF& d1, const QPointF& d2);

    /** @brief 二维叉积(a×b) */
    static double cross2D(const QPointF& a, const QPointF& b);

    QVector<QPointF> m_controlPoints;  ///< 控制点
    Stats m_stats;                      ///< 统计数据
    double m_timeSum = 0.0;            ///< 时间累加器
};
