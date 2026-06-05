/**
 * @file PointLocation.h
 * @brief 平面细分点定位 — 基于slab方法的区域查询
 *
 * 功能: 给定一组线段构建平面细分的slab结构，支持O(log n)的点定位查询。
 *       返回查询点所在的区域编号。适用于嵌入式GIS可视化中的
 *       多边形包含测试和空间分区定位。
 *
 * 协作: GeodesicDistance(距离场) / QuadtreeRegion(区域查询) / LineSweep(线段交点)
 */
#ifndef POINTLOCATION_H
#define POINTLOCATION_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 平面细分点定位器
 *
 * 使用slab(垂直条带)方法将平面按x坐标分割为条带，
 * 每个条带内部按线段排序，支持对数级别的点定位查询。
 */
class PointLocation : public QObject {
    Q_OBJECT

public:
    /** @brief 线段类型: ((x1,y1), (x2,y2)) */
    using Edge = QPair<QPair<double, double>, QPair<double, double>>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalBuilds = 0;       ///< 累计建树次数
        quint64 totalQueries = 0;      ///< 累计查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit PointLocation(QObject* parent = nullptr);
    ~PointLocation();

    /**
     * @brief 从边集构建slab结构
     * @param edges 线段集合，每条边为两个端点
     */
    void build(const QVector<Edge>& edges);

    /**
     * @brief 定位点所在的区域
     * @param x 查询点X坐标
     * @param y 查询点Y坐标
     * @return 区域编号，-1表示未找到
     */
    int locate(double x, double y);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 查询完成信号 @param region 区域编号 */
    void queryCompleted(int region);

private:
    /** @brief slab条带 */
    struct Slab {
        double xLeft;               ///< 左边界x
        double xRight;              ///< 右边界x
        QVector<int> edgeIndices;   ///< 按y排序的边索引
    };

    /** @brief 排序用的线段事件 */
    struct EdgeEvent {
        double y;       ///< 与slab边界的交点y坐标
        int edgeIdx;    ///< 线段索引
    };

    double edgeYAtX(int edgeIdx, double x) const;
    void buildSlabs();
    int countRegions() const;

    QVector<Edge> m_edges;     ///< 存储的线段
    QVector<Slab> m_slabs;     ///< slab条带数组
    Stats m_stats;             ///< 统计信息
    double m_timeSum;          ///< 处理时间累加器
};

#endif // POINTLOCATION_H
