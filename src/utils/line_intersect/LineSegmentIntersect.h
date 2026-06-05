/**
 * @file LineSegmentIntersect.h
 * @brief 线段交集检测 — 简化版Bentley-Ottmann算法
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class LineSegmentIntersect
 * @brief 检测二维线段集合中的所有交点
 *
 * 提供暴力枚举（小规模）和基于扫描线的Bentley-Ottmann简化算法。
 * Point和Segment为轻量值类型，不含QObject开销。
 */
class LineSegmentIntersect : public QObject {
    Q_OBJECT
public:
    /** @brief 二维点 */
    struct Point {
        double x = 0.0; ///< X坐标
        double y = 0.0; ///< Y坐标
    };

    /** @brief 线段 */
    struct Segment {
        Point p1; ///< 起点
        Point p2; ///< 终点
    };

    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalChecks = 0;            ///< 总检查线段对数
        quint64 totalIntersections = 0;      ///< 总发现交点数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit LineSegmentIntersect(QObject* parent = nullptr);

    /**
     * @brief 查找所有线段交点
     * @param segments 线段集合
     * @return 交点列表
     *
     * 线段数 <= 64 时使用暴力枚举；否则使用扫描线简化算法。
     */
    QVector<Point> findIntersections(const QVector<Segment>& segments);

    /**
     * @brief 检测两条线段是否相交
     * @param a 第一条线段
     * @param b 第二条线段
     * @return 是否相交
     */
    bool intersects(const Segment& a, const Segment& b) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 @param intersectionCount 交点数量 */
    void searchCompleted(int intersectionCount);

private:
    mutable Stats  m_stats;
    mutable double m_timeSum = 0.0;

    /**
     * @brief 暴力枚举所有交点
     * @param segments 线段集合
     * @return 交点列表
     */
    QVector<Point> bruteForce(const QVector<Segment>& segments) const;

    /**
     * @brief 扫描线算法查找交点
     * @param segments 线段集合
     * @return 交点列表
     */
    QVector<Point> sweepLine(const QVector<Segment>& segments) const;

    /**
     * @brief 计算两线段交点坐标
     * @param a 第一条线段
     * @param b 第二条线段
     * @return 交点（需先确认相交）
     */
    Point computeIntersection(const Segment& a, const Segment& b) const;

    /**
     * @brief 叉积: (p2-p1) x (p3-p1)
     */
    double cross(const Point& p1, const Point& p2, const Point& p3) const;
};
