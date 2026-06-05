/**
 * @file SegmentTreeRange.h
 * @brief 线段树 — 区间求和查询与单点更新
 *
 * 功能: 构建线段树，支持O(log n)区间求和查询和O(log n)单点更新，
 *       统计查询/更新次数/耗时。
 */
#ifndef SEGMENTTREERANGE_H
#define SEGMENTTREERANGE_H

#include <QObject>
#include <QVector>

/**
 * @class SegmentTreeRange
 * @brief 线段树，专注于区间求和查询与单点更新
 *
 * 标准线段树实现，使用数组存储完全二叉树结构。
 */
class SegmentTreeRange : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalQueries = 0;       ///< 总查询次数
        quint64 totalUpdates = 0;       ///< 总更新次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit SegmentTreeRange(QObject* parent = nullptr);

    /**
     * @brief 从数组构建线段树
     * @param data 输入数据数组
     */
    void build(QVector<double> data);

    /**
     * @brief 区间求和查询 [left, right]
     * @param left 左边界(含)
     * @param right 右边界(含)
     * @return 区间求和结果
     */
    double rangeSum(int left, int right);

    /**
     * @brief 单点更新
     * @param index 索引位置
     * @param value 新值(替换而非增量)
     */
    void pointUpdate(int index, double value);

    /** @brief 获取统计信息 */
    const Stats& stats() { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 获取原始数据大小 */
    int size() { return m_size; }

    /** @brief 获取原始数据(只读) */
    const QVector<double>& data() { return m_data; }

signals:
    /** @brief 查询完成信号 @param result 查询结果 */
    void queryCompleted(double result);

private:
    void buildImpl(int node, int start, int end);
    double queryImpl(int node, int start, int end, int l, int r);
    void updateImpl(int node, int start, int end, int idx, double val);

    int             m_size;     ///< 原始数据大小
    QVector<double> m_data;     ///< 原始数据副本
    QVector<double> m_tree;     ///< 线段树数组
    Stats           m_stats;    ///< 统计信息
    double          m_timeSum;  ///< 累计耗时
};

#endif // SEGMENTTREERANGE_H
