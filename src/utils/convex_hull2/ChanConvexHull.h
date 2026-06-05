/**
 * @file ChanConvexHull.h
 * @brief Chan算法凸包 — O(n log h)最优二维凸包
 *
 * 功能: 实现Chan的凸包算法，对于n个点中h个凸包点的情况
 *       达到O(n log h)时间复杂度，优于Graham扫描的O(n log n)。
 *       统计计算次数/耗时。
 */
#ifndef CHANCONVEXHULL_H
#define CHANCONVEXHULL_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class ChanConvexHull
 * @brief Chan算法凸包计算器，O(n log h)时间复杂度
 *
 * 将点集分成若干子集，分别用Graham扫描求子凸包，
 * 再用Jarvis步进法合并。自动猜测凸包大小h并验证。
 */
class ChanConvexHull : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalComputations = 0;   ///< 总计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit ChanConvexHull(QObject* parent = nullptr);

    /**
     * @brief 计算二维点集的凸包
     * @param points 输入二维点集
     * @return 凸包顶点的索引(按逆时针排列)
     */
    QVector<int> compute2d(QVector<QPair<double,double>> points);

    /** @brief 获取统计信息 */
    const Stats& stats() { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param hullSize 凸包顶点数量 */
    void computationCompleted(int hullSize);

private:
    /** Graham扫描法求子集凸包 */
    QVector<int> grahamScan(
        const QVector<QPair<double,double>>& pts,
        const QVector<int>& indices);

    /** Jarvis步进法合并子凸包 */
    QVector<int> jarvisMerge(
        const QVector<QPair<double,double>>& pts,
        const QVector<QVector<int>>& subHulls,
        int maxSteps);

    double cross(const QPair<double,double>& o,
                 const QPair<double,double>& a,
                 const QPair<double,double>& b);

    Stats  m_stats;    ///< 统计信息
    double m_timeSum;  ///< 累计耗时
};

#endif // CHANCONVEXHULL_H
