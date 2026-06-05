/**
 * @file PolygonTriangulation.h
 * @brief 多边形三角剖分 — 耳切法(Ear Clipping)
 *
 * 功能: 使用耳切法将简单多边形分解为三角形扇，
 *       支持凹多边形，统计剖分次数/耗时。
 */
#ifndef POLYGONTRIANGULATION_H
#define POLYGONTRIANGULATION_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class PolygonTriangulation
 * @brief 多边形三角剖分器，耳切法实现
 *
 * 耳切法(Ear Clipping)将简单多边形分解为n-2个三角形。
 * 每一步找到一个"耳朵"(凸顶点，其对角线在多边形内部)，
 * 切掉这个耳朵，直到只剩一个三角形。
 */
class PolygonTriangulation : public QObject {
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
    explicit PolygonTriangulation(QObject* parent = nullptr);

    /**
     * @brief 对多边形进行三角剖分
     * @param polygon 多边形顶点列表(按顺时针或逆时针排列)
     * @return 三角形列表，每个三角形为三个顶点索引
     */
    QVector<QPair<int,int,int>> triangulate(
        QVector<QPair<double,double>> polygon);

    /** @brief 获取统计信息 */
    const Stats& stats() { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 三角剖分完成信号 @param triangleCount 三角形数量 */
    void triangulationCompleted(int triangleCount);

private:
    /** 判断点p是否在三角形(p0,p1,p2)内 */
    bool pointInTriangle(const QPair<double,double>& p,
                         const QPair<double,double>& p0,
                         const QPair<double,double>& p1,
                         const QPair<double,double>& p2);

    /** 叉积 */
    double cross(const QPair<double,double>& o,
                 const QPair<double,double>& a,
                 const QPair<double,double>& b);

    /** 判断顶点i是否为耳朵 */
    bool isEar(const QVector<QPair<double,double>>& polygon,
               const QVector<int>& indices, int i);

    Stats  m_stats;    ///< 统计信息
    double m_timeSum;  ///< 累计耗时
};

#endif // POLYGONTRIANGULATION_H
