/**
 * @file DataInterpolator.h
 * @brief 数据插值引擎 — 线性/样条/最近邻/多项式插值
 *
 * 功能: 支持线性/三次样条/最近邻/Lagrange多项式4种插值方法，
 *       对稀疏数据点进行插值填充，用于波形重建。
 *
 * 协作: DataSynchronizer(时间对齐后插值) / ChartWidget(波形填充)
 */
#ifndef DATAINTERPOLATOR_H
#define DATAINTERPOLATOR_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <QMap>

/**
 * @brief 数据插值引擎 — 多方法插值计算
 */
class DataInterpolator : public QObject {
    Q_OBJECT

public:
    /** @brief 插值方法 */
    enum class InterpolationMethod {
        Linear,     ///< 线性插值
        CubicSpline,///< 三次样条插值
        Nearest,    ///< 最近邻插值
        Polynomial  ///< Lagrange多项式插值
    };
    Q_ENUM(InterpolationMethod)

    /** @brief 统计 */
    struct Stats {
        quint64 totalInterpolations = 0;   ///< 累计插值次数
        double  averageError = 0.0;        ///< 平均误差
        double  peakError = 0.0;           ///< 峰值误差
        quint64 cacheHits = 0;             ///< 缓存命中
        quint64 cacheMisses = 0;           ///< 缓存未命中
    };

    explicit DataInterpolator(QObject* parent = nullptr);

    /** @brief 设置插值方法 @param method 方法 */
    void setMethod(InterpolationMethod method);

    /** @brief 设置控制点 @param xPoints X坐标 @param yPoints Y坐标 */
    void setControlPoints(const QVector<double>& xPoints,
                          const QVector<double>& yPoints);

    /** @brief 在指定X处插值 @param x 目标X @return 插值Y值 */
    double interpolate(double x);

    /** @brief 批量插值 @param xValues X坐标列表 @return Y值列表 */
    QVector<double> interpolateBatch(const QVector<double>& xValues);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插值完成 @param count 插值点数 */
    void interpolationComplete(int count);

private:
    double interpolateLinear(double x) const;
    double interpolateCubic(double x) const;
    double interpolateNearest(double x) const;
    double interpolatePolynomial(double x) const;

    int findSegment(double x) const;

    InterpolationMethod m_method;       ///< 插值方法
    QVector<double> m_xPoints;          ///< 控制点X坐标
    QVector<double> m_yPoints;          ///< 控制点Y坐标
    QVector<double> m_cubicCoeffs;      ///< 样条系数

    Stats m_stats;
    double m_errorSum;                  ///< 误差累加器
};

#endif // DATAINTERPOLATOR_H
