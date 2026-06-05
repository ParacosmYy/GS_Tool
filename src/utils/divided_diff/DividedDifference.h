/**
 * @file DividedDifference.h
 * @brief 牛顿差商表 — 多项式插值
 *
 * 功能: 构造 Newton 差商表并利用其进行任意点插值。
 *       支持任意分布的节点(不等距)。
 *
 * 协作: LagrangeInterpolation(拉格朗日) / ChebyshevApproximation(最佳逼近)
 */
#ifndef DIVIDEDIFFERENCE_H
#define DIVIDEDIFFERENCE_H

#include <QObject>
#include <QVector>

/**
 * @brief 牛顿差商插值器
 */
class DividedDifference : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInterpolations = 0;  ///< 累计插值次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit DividedDifference(QObject* parent = nullptr);

    /**
     * @brief 构建 Newton 差商表
     * @param xData 节点横坐标
     * @param yData 节点纵坐标
     * @return 下三角差商表 table[i][j] = f[x_i,...,x_{i+j}]
     */
    QVector<QVector<double>> buildTable(QVector<double> xData,
                                        QVector<double> yData);

    /**
     * @brief 利用差商表在 xQuery 处求值
     * @param xData 节点横坐标
     * @param table 差商表(由 buildTable 返回)
     * @param xQuery 查询点
     * @return 插值结果
     */
    double interpolate(QVector<double> xData,
                       QVector<QVector<double>> table,
                       double xQuery);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插值完成 @param pointCount 节点数 */
    void interpolationCompleted(int pointCount);

private:
    Stats  m_stats;          ///< 统计信息
    double m_timeSum = 0.0;  ///< 累计耗时
};

#endif // DIVIDEDIFFERENCE_H
