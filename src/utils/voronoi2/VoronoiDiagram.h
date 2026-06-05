/**
 * @file VoronoiDiagram.h
 * @brief Voronoi图 — 基于Delaunay三角剖分的对偶图
 *
 * 功能: 从Delaunay三角剖分计算Voronoi图多边形，
 *       统计计算次数/耗时。
 *
 * @note 此类与 utils/voronoi/VoronoiDiagram 不同:
 *       voronoi/ 使用Fortune扫描线暴力法；
 *       本类使用Delaunay对偶法，输出为Voronoi多边形。
 */
#ifndef VORONOI_DIAGRAM_DUAL_H
#define VORONOI_DIAGRAM_DUAL_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class VoronoiDual
 * @brief Voronoi图计算器，基于Delaunay三角剖分的对偶图
 *
 * 先构建Delaunay三角剖分，再取三角形外接圆心作为Voronoi
 * 顶点，连接共享边的三角形外接圆心形成Voronoi边。
 */
class VoronoiDual : public QObject {
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
    explicit VoronoiDual(QObject* parent = nullptr);

    /**
     * @brief 从站点集合计算Voronoi图
     * @param sites 站点坐标列表
     * @return 每个站点的Voronoi多边形顶点列表(按逆时针排列)
     */
    QVector<QVector<QPair<double,double>>> compute(
        QVector<QPair<double,double>> sites);

    /** @brief 获取统计信息 */
    const Stats& stats() { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param siteCount 站点数量 */
    void computationCompleted(int siteCount);

private:
    /** 三角形外接圆心计算 */
    QPair<double,double> circumcenter(
        const QPair<double,double>& p0,
        const QPair<double,double>& p1,
        const QPair<double,double>& p2);

    Stats  m_stats;    ///< 统计信息
    double m_timeSum;  ///< 累计耗时
};

#endif // VORONOI_DIAGRAM_DUAL_H
