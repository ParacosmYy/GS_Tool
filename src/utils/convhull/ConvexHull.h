/**
 * @file ConvexHull.h
 * @brief 凸包算法 — Graham Scan + Andrew's Monotone Chain
 *
 * 功能: 计算点集凸包，点在凸包内判断，Shoelace面积公式，
 *       统计计算次数/耗时，凸包完成信号。
 */
#ifndef CONVEXHULL_H
#define CONVEXHULL_H

#include <QObject>
#include <QVector>

class ConvexHull : public QObject {
    Q_OBJECT
public:
    /** 二维点 */
    struct Point {
        double x = 0.0;  ///< X坐标
        double y = 0.0;  ///< Y坐标
    };

    /** 操作统计 */
    struct Stats {
        quint64 totalComputations = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit ConvexHull(QObject* parent = nullptr);

    /** @brief 计算凸包(Andrew's Monotone Chain) @param points 输入点集 @return 凸包顶点(逆时针) */
    QVector<Point> compute(const QVector<Point>& points);

    /** @brief 判断点是否在凸包内 @param point 待测点 @return 是否在内部 */
    bool isInside(const Point& point);

    /** @brief 计算凸包面积(Shoelace公式) @return 面积，若未计算凸包则返回0 */
    double hullArea();

    /** @brief 获取上次计算的凸包 */
    const QVector<Point>& lastHull() const { return m_hull; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 凸包计算完成 @param vertices 顶点数 @param area 面积 */
    void hullComputed(int vertices, double area);

private:
    static double cross(const Point& o, const Point& a, const Point& b);
    static double shoelaceArea(const QVector<Point>& hull);

    QVector<Point> m_hull;   ///< 最近一次计算的凸包
    Stats m_stats;
    double m_timeSum;
};

#endif // CONVEXHULL_H
