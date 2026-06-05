/**
 * @file ConvexHull3D.h
 * @brief 三维凸包 — QuickHull算法实现
 *
 * 功能: 给定三维点集, 计算其凸包面片和边, 支持点包含测试、
 *       体积/表面积计算、凸包简化等功能。
 *
 * 协作: DataNormalizer(坐标归一化) / DataQualityScorer(质量评估)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 三维凸包计算器
 *
 * 使用QuickHull算法在O(n log n)期望时间内计算三维凸包,
 * 输出为三角形面片列表(每面3个顶点索引)。
 */
class ConvexHull3D : public QObject
{
    Q_OBJECT

public:
    /** @brief 三维点 */
    struct Point3D {
        double x = 0.0, y = 0.0, z = 0.0;

        Point3D() = default;
        Point3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

        Point3D operator-(const Point3D& o) const {
            return {x - o.x, y - o.y, z - o.z};
        }
        Point3D operator+(const Point3D& o) const {
            return {x + o.x, y + o.y, z + o.z};
        }
        Point3D operator*(double s) const {
            return {x * s, y * s, z * s};
        }
        double dot(const Point3D& o) const {
            return x * o.x + y * o.y + z * o.z;
        }
        Point3D cross(const Point3D& o) const {
            return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
        }
        double length() const {
            return std::sqrt(x * x + y * y + z * z);
        }
    };

    /** @brief 三角形面片(3个顶点索引) */
    struct Face {
        int a, b, c;       ///< 顶点索引
        Point3D normal;     ///< 面法线
        double area = 0.0;  ///< 面面积

        Face() : a(0), b(0), c(0) {}
        Face(int a_, int b_, int c_) : a(a_), b(b_), c(c_) {}
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalHullsComputed = 0;         ///< 累计计算凸包次数
        int totalInputPoints = 0;           ///< 累计输入点数
        int totalFaces = 0;                 ///< 累计面片数
        int totalContainmentTests = 0;      ///< 累计包含测试次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit ConvexHull3D(QObject* parent = nullptr);

    /**
     * @brief 计算三维凸包
     * @param points 输入点集(至少4个非共面点)
     * @return 凸包面片列表
     */
    QVector<Face> compute(const QVector<Point3D>& points);

    /**
     * @brief 测试点是否在凸包内部
     * @param point 待测试点
     * @param points 原始点集
     * @param faces 凸包面片
     * @return true=在内部或边界上
     */
    bool contains(const Point3D& point,
                  const QVector<Point3D>& points,
                  const QVector<Face>& faces) const;

    /**
     * @brief 计算凸包体积
     * @param points 原始点集
     * @param faces 凸包面片
     * @return 体积
     */
    double volume(const QVector<Point3D>& points,
                  const QVector<Face>& faces) const;

    /**
     * @brief 计算凸包表面积
     * @param faces 凸包面片
     * @return 表面积
     */
    double surfaceArea(const QVector<Face>& faces) const;

    /**
     * @brief 获取凸包的质心
     * @param points 原始点集
     * @param faces 凸包面片
     * @return 质心坐标
     */
    Point3D centroid(const QVector<Point3D>& points,
                     const QVector<Face>& faces) const;

    /**
     * @brief 获取凸包顶点索引(去重)
     * @param faces 凸包面片
     * @return 顶点索引列表
     */
    QVector<int> hullVertices(const QVector<Face>& faces) const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 凸包计算完成 @param vertexCount 顶点数 @param faceCount 面片数 */
    void hullComputed(int vertexCount, int faceCount);

private:
    /**
     * @brief 计算面法线和面积
     * @param face 面片
     * @param points 点集
     */
    void computeFaceNormal(Face& face, const QVector<Point3D>& points) const;

    /**
     * @brief 构建初始四面体
     * @param points 输入点集
     * @return 初始4个面片, 失败返回空
     */
    QVector<Face> buildInitialTetrahedron(const QVector<Point3D>& points);

    Stats m_stats;
    double m_timeSum = 0.0;
};
