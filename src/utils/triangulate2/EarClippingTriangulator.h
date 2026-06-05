/**
 * @file EarClippingTriangulator.h
 * @brief Ear-Clipping多边形三角剖分
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Ear-Clipping三角剖分器
 *
 * 将简单多边形分解为三角形扇,
 * 支持带孔多边形和面积/法线计算。
 */
class EarClippingTriangulator : public QObject
{
    Q_OBJECT

public:
    /** @brief 二维点 */
    struct Point {
        double x = 0.0, y = 0.0;
    };

    /** @brief 三角形 */
    struct Triangle {
        int a, b, c; ///< 顶点索引
        double area = 0.0;
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalTriangulations = 0;   ///< 总三角剖分次数
        int totalTriangles = 0;        ///< 总三角形数
        int totalVerticesProcessed = 0; ///< 总顶点数
        double avgProcessingTimeMs = 0.0;
    };

    explicit EarClippingTriangulator(QObject* parent = nullptr);

    /**
     * @brief 对简单多边形进行三角剖分
     * @param vertices 多边形顶点(按顺时针或逆时针顺序)
     * @return 三角形列表
     */
    QVector<Triangle> triangulate(const QVector<Point>& vertices);

    /**
     * @brief 计算多边形面积
     * @param vertices 多边形顶点
     * @return 有向面积(正值=逆时针)
     */
    double polygonArea(const QVector<Point>& vertices) const;

    /**
     * @brief 检查点是否在三角形内
     */
    bool pointInTriangle(const Point& p, const Point& a,
                          const Point& b, const Point& c) const;

    /**
     * @brief 计算三角形面积
     */
    double triangleArea(const Point& a, const Point& b,
                         const Point& c) const;

    /**
     * @brief 检查多边形是否为凸的
     */
    bool isConvex(const QVector<Point>& vertices) const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 三角剖分完成信号 */
    void triangulationCompleted(int vertexCount, int triangleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    double cross2d(const Point& o, const Point& a, const Point& b) const;
    bool isConvexVertex(const Point& prev, const Point& curr,
                         const Point& next, bool ccw) const;
    bool isEar(const QVector<Point>& poly, const QVector<int>& indices,
               int i, bool ccw) const;
};
