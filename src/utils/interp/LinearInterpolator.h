/**
 * @file LinearInterpolator.h
 * @brief 线性插值器 — 分段线性插值与反插值
 *
 * 功能: 对有序控制点集合执行分段线性插值，支持批量插值
 *       和反插值(从Y求X)。适用于传感器校准曲线、
 *       非线性变换表、温度-电压映射等场景。
 *
 * 协作: DataInterpolator(通用插值) / DataSynchronizer(时间对齐)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QElapsedTimer>

/**
 * @brief 线性插值器 — 分段线性插值与反插值
 *
 * 输入控制点必须按X坐标升序排列(自动排序)，
 * 在相邻控制点之间执行线性插值。
 */
class LinearInterpolator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInterpolations = 0;    ///< 累计插值次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit LinearInterpolator(QObject* parent = nullptr);

    /**
     * @brief 设置控制点集合
     * @param points (x, y)点对列表，将按x升序排序
     */
    void setPoints(const QVector<QPair<double, double>>& points);

    /**
     * @brief 在指定x处执行线性插值
     * @param x 目标X坐标
     * @return 插值结果Y值；越界时返回最近端点值
     */
    double interpolate(double x) const;

    /**
     * @brief 批量线性插值
     * @param xs 目标X坐标列表
     * @return 对应Y值列表
     */
    QVector<double> interpolateBatch(const QVector<double>& xs) const;

    /**
     * @brief 反插值 — 给定Y值求对应X值
     * @param y 目标Y值
     * @return 第一个满足条件的X值；若无法求解返回NaN
     */
    double inverseInterpolate(double y) const;

    /** @brief 获取控制点数量 */
    int pointCount() const { return m_points.size(); }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 插值完成信号 @param count 插值点数 */
    void interpolationComplete(int count);

private:
    /**
     * @brief 二分查找x所属的段索引
     * @param x 目标X值
     * @return 段索引i，使得 points[i].x <= x < points[i+1].x
     */
    int findSegment(double x) const;

    QVector<QPair<double, double>> m_points;  ///< 控制点(已按x排序)

    mutable QElapsedTimer m_timer;   ///< 计时器
    mutable double  m_timeSum;       ///< 累计耗时
    mutable Stats   m_stats;         ///< 统计信息
};
