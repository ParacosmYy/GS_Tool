/**
 * @file ConvexHull3D.h
 * @brief 三维凸包(Gift Wrapping)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class ConvexHull3D
 * @brief 三维凸包 — 在3D空间中计算点集的凸包
 *
 * 使用增量法构建三维凸包，支持体积和表面积计算。
 * 适用于3D几何、碰撞检测、包围盒等场景。
 */
class ConvexHull3D : public QObject
{
    Q_OBJECT

public:
    /** @brief 三维点 */
    struct Point3D {
        double x, y, z;
    };

    /** @brief 三角面片 */
    struct Face {
        int v0, v1, v2;  /**< 顶点索引 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalComputed = 0;     /**< 总计算次数 */
        int totalFaces = 0;        /**< 总面片数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit ConvexHull3D(QObject* parent = nullptr);

    /**
     * @brief 计算凸包
     * @param points 3D点集
     * @return 面片列表
     */
    QVector<Face> compute(const QVector<Point3D>& points);

    /**
     * @brief 计算凸包体积
     * @param points 点集
     * @param faces 面片
     * @return 体积
     */
    double volume(const QVector<Point3D>& points,
                   const QVector<Face>& faces) const;

    /**
     * @brief 计算凸包表面积
     * @param points 点集
     * @param faces 面片
     * @return 表面积
     */
    double surfaceArea(const QVector<Point3D>& points,
                        const QVector<Face>& faces) const;

    /**
     * @brief 检测点是否在凸包内部
     * @param point 测试点
     * @param hullPoints 凸包顶点
     * @param faces 凸包面片
     * @return 是否在内部
     */
    bool isInside(const Point3D& point,
                   const QVector<Point3D>& hullPoints,
                   const QVector<Face>& faces) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void computationCompleted(int faceCount);

private:
    double tripleProduct(const Point3D& a, const Point3D& b,
                          const Point3D& c, const Point3D& d) const;
    double triangleArea(const Point3D& a, const Point3D& b,
                         const Point3D& c) const;

    Stats m_stats;
    double m_timeSum;
};
