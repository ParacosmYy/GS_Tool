/**
 * @file GrevilleAbscissae.h
 * @brief Greville横坐标计算 — B样条节点向量的平均坐标
 *
 * 功能: 给定B样条节点向量 (knot vector) 和次数(degree)，
 *       计算对应的Greville横坐标(又称节点均值)。
 *       这些点在B样条插值和逼近中作为参数化的自然选择。
 *
 * 协作: BernsteinPolynomial(Bernstein基) / LagrangeInterpolation(插值)
 */
#ifndef GREVILLEABSCISSAE_H
#define GREVILLEABSCISSAE_H

#include <QObject>
#include <QVector>

/**
 * @brief Greville横坐标计算器
 *
 * Greville横坐标公式: xi_i = (knots[i+1] + ... + knots[i+degree]) / degree
 * 数量 = knots.size() - degree - 1（即控制点数）。
 */
class GrevilleAbscissae : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息结构 */
    struct Stats {
        quint64 totalComputations = 0;   ///< 累计计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit GrevilleAbscissae(QObject* parent = nullptr);

    /**
     * @brief 计算Greville横坐标
     * @param knots 节点向量(须非递减，长度 >= degree + 2)
     * @param degree B样条次数(须 >= 1)
     * @return Greville横坐标向量
     */
    QVector<double> compute(const QVector<double>& knots, int degree);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /**
     * @brief 计算完成信号
     * @param pointCount 输出点数
     */
    void computationCompleted(int pointCount);

private:
    Stats m_stats;      ///< 统计信息
    double m_timeSum;   ///< 处理时间累加器
};

#endif // GREVILLEABSCISSAE_H
