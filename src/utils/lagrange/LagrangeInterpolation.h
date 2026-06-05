/**
 * @file LagrangeInterpolation.h
 * @brief Lagrange多项式插值 — 经典插值方法
 *
 * 功能: 使用Lagrange基函数构造插值多项式，
 *       支持任意散点数据的插值计算。
 *
 * 协作: CubicInterpolator(三次样条) / HermiteInterpolation(Hermite插值)
 */
#ifndef LAGRANGEINTERPOLATION_H
#define LAGRANGEINTERPOLATION_H

#include <QObject>
#include <QVector>

/**
 * @brief Lagrange多项式插值
 */
class LagrangeInterpolation : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalInterpolations = 0;    ///< 累计插值次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit LagrangeInterpolation(QObject* parent = nullptr);

    /**
     * @brief 执行Lagrange插值
     * @param x 已知节点x坐标
     * @param y 已知节点y坐标
     * @param xQuery 待查询的x坐标集合
     * @return 插值结果y坐标集合
     */
    QVector<double> interpolate(const QVector<double>& x,
                                const QVector<double>& y,
                                const QVector<double>& xQuery);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插值完成 @param pointCount 查询点数量 */
    void interpolationCompleted(int pointCount);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // LAGRANGEINTERPOLATION_H
