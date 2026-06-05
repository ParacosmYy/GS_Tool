/**
 * @file DelaunayFlip.h
 * @brief Delaunay三角剖分 — 增量插入+边翻转
 *
 * 功能: 逐点插入构建Delaunay三角剖分，通过Lawson边翻转
 *       算法维护Delaunay性质，统计剖分次数/耗时。
 */
#ifndef DELAUNAYFLIP_H
#define DELAUNAYFLIP_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class DelaunayFlip
 * @brief Delaunay三角剖分器，增量插入+Lawson边翻转
 *
 * 使用超级三角形包裹所有点，逐点插入后通过边翻转恢复
 * Delaunay性质，最终移除与超级三角形关联的三角形。
 */
class DelaunayFlip : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalTriangulations = 0;   ///< 总剖分次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit DelaunayFlip(QObject* parent = nullptr);

    /**
     * @brief 对点集执行Delaunay三角剖分
     * @param points 输入二维点集
     * @return 三角形列表，每个三角形为三个顶点索引
     */
    QVector<QPair<int,int,int>> triangulate(
        QVector<QPair<double,double>> points);

    /** @brief 获取统计信息 */
    const Stats& stats() { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 三角剖分完成信号
     * @param triangleCount 三角形数量
     * @param pointCount 点数量
     */
    void triangulationCompleted(int triangleCount, int pointCount);

private:
    /** 三角形(存储顶点索引，-1表示超级三角形虚拟点) */
    struct Tri {
        int a, b, c;           ///< 顶点索引
        int adjA, adjB, adjC;  ///< 对边邻居三角形索引(-1无邻居)
    };

    /** 边(由两个顶点索引定义) */
    struct Edge {
        int v1, v2; ///< 顶点索引
    };

    double crossProduct(const QPair<double,double>& p0,
                        const QPair<double,double>& p1,
                        const QPair<double,double>& p2);
    bool   inCircumcircle(int pi, int ti);
    void   computeCircumcircle(int ti);
    void   flipEdge(int ti, int ei);
    void   insertPoint(int pi);
    void   removeSuperTriangle();

    QVector<QPair<double,double>> m_points;     ///< 原始点集+超级三角形点
    QVector<Tri>    m_tris;                      ///< 三角形列表
    QVector<double> m_ccX, m_ccY, m_ccR2;       ///< 外接圆心X/Y/半径平方
    Stats           m_stats;                     ///< 统计信息
    double          m_timeSum;                   ///< 累计耗时
};

#endif // DELAUNAYFLIP_H
