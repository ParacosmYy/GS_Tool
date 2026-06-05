/**
 * @file LineSweepIntersect.h
 * @brief 线段交点检测 — Bentley-Ottmann扫描线算法
 *
 * 功能: 使用扫描线算法高效检测一组线段之间的所有交点，
 *       O((n+k)log n)时间复杂度，统计计算次数/耗时。
 */
#ifndef LINESWEEPINTERSECT_H
#define LINESWEEPINTERSECT_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class LineSweepIntersect
 * @brief 线段交点检测器，Bentley-Ottmann扫描线算法
 *
 * 通过水平扫描线从上到下(或从左到右)扫过所有线段，
 * 维护活跃线段的有序集合，检测相邻线段的交点。
 */
class LineSweepIntersect : public QObject {
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
    explicit LineSweepIntersect(QObject* parent = nullptr);

    /**
     * @brief 查找所有线段交点
     * @param segments 线段列表，每条线段由两个端点定义
     * @return 交点坐标列表
     */
    QVector<QPair<double,double>> findIntersections(
        QVector<QPair<QPair<double,double>,QPair<double,double>>> segments);

    /** @brief 获取统计信息 */
    const Stats& stats() { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 @param intersectionCount 交点数量 */
    void computationCompleted(int intersectionCount);

private:
    /** 计算两条线段的交点 */
    bool segmentIntersect(
        const QPair<double,double>& a1, const QPair<double,double>& a2,
        const QPair<double,double>& b1, const QPair<double,double>& b2,
        QPair<double,double>& intersection);

    Stats  m_stats;    ///< 统计信息
    double m_timeSum;  ///< 累计耗时
};

#endif // LINESWEEPINTERSECT_H
