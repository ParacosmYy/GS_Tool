/**
 * @file VoronoiDiagram.h
 * @brief Voronoi图 — Fortune's sweep line(简化版)
 *
 * 功能: 从点集计算Voronoi图，最近邻站点查询，
 *       统计计算/查询次数/耗时，计算完成信号。
 */
#ifndef VORONOIDIAGRAM_H
#define VORONOIDIAGRAM_H

#include <QObject>
#include <QVector>

class VoronoiDiagram : public QObject {
    Q_OBJECT
public:
    /** 二维点 */
    struct Point {
        double x = 0.0;  ///< X坐标
        double y = 0.0;  ///< Y坐标
    };

    /** Voronoi边 */
    struct Edge {
        Point a;       ///< 边端点A
        Point b;       ///< 边端点B
        int site1 = -1; ///< 关联站点1索引
        int site2 = -1; ///< 关联站点2索引
    };

    /** 操作统计 */
    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalQueries = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit VoronoiDiagram(QObject* parent = nullptr);

    /** @brief 从点集计算Voronoi图 @param sites 站点点集 */
    void compute(const QVector<Point>& sites);

    /** @brief 查询最近站点 @param x X坐标 @param y Y坐标 @return 最近站点索引(-1表示无数据) */
    int nearestSite(double x, double y);

    /** @brief 获取计算出的边 */
    const QVector<Edge>& edges() const { return m_edges; }

    /** @brief 获取站点 */
    const QVector<Point>& sites() const { return m_sites; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 图计算完成 @param sites 站点数 @param edges 边数 */
    void diagramComputed(int sites, int edges);

private:
    void computeBruteForce();

    QVector<Point> m_sites;   ///< 站点点集
    QVector<Edge> m_edges;    ///< Voronoi边
    Stats m_stats;
    double m_timeSum;
};

#endif // VORONOIDIAGRAM_H
