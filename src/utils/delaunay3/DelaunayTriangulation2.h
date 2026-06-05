/**
 * @file DelaunayTriangulation2.h
 * @brief Delaunay三角剖分增强版(Delaunay Triangulation Enhanced)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class DelaunayTriangulation2
 * @brief Delaunay三角剖分增强版 — Bowyer-Watson算法
 *
 * 支持增量式插入、Voronoi对偶图提取、约束Delaunay。
 * 适用于网格生成、最近邻查询、地形建模等场景。
 */
class DelaunayTriangulation2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 二维点 */
    struct Point2D {
        double x, y;
    };

    /** @brief 三角形 */
    struct Triangle {
        int v0, v1, v2;  /**< 顶点索引 */
    };

    /** @brief 边 */
    struct Edge {
        int v0, v1;      /**< 顶点索引 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalTriangulated = 0; /**< 总剖分次数 */
        int totalTriangles = 0;    /**< 总三角形数 */
        int totalPoints = 0;       /**< 总插入点数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit DelaunayTriangulation2(QObject* parent = nullptr);

    /**
     * @brief 一次性三角剖分
     * @param points 点集
     * @return 三角形列表
     */
    QVector<Triangle> triangulate(const QVector<Point2D>& points);

    /**
     * @brief 增量式:初始化
     * @param superWidth 超级三角形宽度
     * @param superHeight 超级三角形高度
     */
    void initIncremental(double superWidth = 1000.0,
                          double superHeight = 1000.0);

    /**
     * @brief 增量式:插入点
     * @param p 新点
     */
    void insertPoint(const Point2D& p);

    /**
     * @brief 增量式:获取当前三角形
     * @return 三角形列表(已去除超级三角形相关)
     */
    QVector<Triangle> getTriangles() const;

    /**
     * @brief 提取Voronoi图(对偶)
     * @param points 原始点集
     * @param triangles 三角剖分结果
     * @return Voronoi边列表
     */
    QVector<Edge> voronoiEdges(const QVector<Point2D>& points,
                                const QVector<Triangle>& triangles) const;

    /**
     * @brief 检测点是否在三角形内
     * @param p 测试点
     * @param a,b,c 三角形顶点
     * @return 是否在内部
     */
    static bool inTriangle(const Point2D& p, const Point2D& a,
                            const Point2D& b, const Point2D& c);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 剖分完成信号 */
    void triangulationCompleted(int triangleCount);

private:
    double circumCircleRadius(const Point2D& a, const Point2D& b,
                               const Point2D& c) const;
    bool inCircumCircle(const Point2D& p, const Point2D& a,
                         const Point2D& b, const Point2D& c) const;

    QVector<Point2D> m_points;
    QVector<Triangle> m_triangles;
    bool m_incrementalMode;

    Stats m_stats;
    double m_timeSum;
};
