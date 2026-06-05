/**
 * @file GeodesicDistance.h
 * @brief 测地距离计算 — 基于代价地图的Dijkstra最短路径
 *
 * 功能: 给定二维代价地图，计算从起点到所有网格点的测地距离。
 *       使用优先队列优化的Dijkstra算法，适用于嵌入式数据可视化
 *       中的路径规划和距离场计算。
 *
 * 协作: QuadtreeRegion(区域查询) / KdTreeBalancer(空间索引) / DistanceMatrix(距离矩阵)
 */
#ifndef GEODESICDISTANCE_H
#define GEODESICDISTANCE_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 测地距离计算器
 *
 * 在二维代价地图上执行Dijkstra算法，输出距离场。
 * 代价为零或负值的网格视为不可通行障碍。
 */
class GeodesicDistance : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations = 0;  ///< 累计计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit GeodesicDistance(QObject* parent = nullptr);
    ~GeodesicDistance();

    /**
     * @brief 计算测地距离场
     * @param costMap 二维代价地图，每个值代表经过该格的代价
     * @param startX 起点X坐标(列)
     * @param startY 起点Y坐标(行)
     * @return 距离场矩阵，不可达点距离为-1
     */
    QVector<QVector<double>> compute(
        const QVector<QVector<double>>& costMap,
        int startX, int startY);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param width 地图宽度 @param height 地图高度 */
    void computationCompleted(int width, int height);

private:
    /** @brief 网格方向偏移量(8邻域) */
    static constexpr int s_dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static constexpr int s_dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static constexpr double s_diagCost = 1.4142135623730951; ///< sqrt(2)

    Stats m_stats;      ///< 统计信息
    double m_timeSum;    ///< 处理时间累加器
};

#endif // GEODESICDISTANCE_H
