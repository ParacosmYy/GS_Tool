/**
 * @file RationalInterpolation.h
 * @brief 有理函数插值 — 重心形式
 *
 * 功能: 利用重心有理插值公式(Berrut 中值型)对任意分布节点
 *       进行快速、稳定的插值，避免 Runge 现象。
 *
 * 协作: DividedDifference(多项式) / ChebyshevApproximation(最佳逼近)
 */
#ifndef RATIONALINTERPOLATION_H
#define RATIONALINTERPOLATION_H

#include <QObject>
#include <QVector>

/**
 * @brief 重心有理插值器
 */
class RationalInterpolation : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInterpolations = 0;  ///< 累计插值次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit RationalInterpolation(QObject* parent = nullptr);

    /**
     * @brief 对一组查询点执行重心有理插值
     * @param xData  节点横坐标(n 个)
     * @param yData  节点纵坐标(n 个)
     * @param xQuery 查询横坐标(m 个)
     * @return 插值结果(m 个)
     */
    QVector<double> interpolate(QVector<double> xData,
                                QVector<double> yData,
                                QVector<double> xQuery);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插值完成 @param pointCount 查询点数 */
    void interpolationCompleted(int pointCount);

private:
    Stats  m_stats;          ///< 统计信息
    double m_timeSum = 0.0;  ///< 累计耗时
};

#endif // RATIONALINTERPOLATION_H
