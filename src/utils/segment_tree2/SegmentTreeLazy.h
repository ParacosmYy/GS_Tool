/**
 * @file SegmentTreeLazy.h
 * @brief 懒标记线段树 — 支持区间更新和区间查询
 *
 * 功能: 实现带懒传播(Lazy Propagation)的线段树，
 *       支持O(logN)的区间加/区间赋值/区间最值/区间求和操作。
 *       适用于数据流分析中的滑动窗口统计。
 *
 * 协作: SlidingWindowStats(滑动窗口) / WindowedAggregator(窗口聚合)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <vector>
#include <limits>

/**
 * @brief 懒标记线段树 — 区间更新+区间查询
 */
class SegmentTreeLazy : public QObject {
    Q_OBJECT

public:
    /** @brief 操作类型 */
    enum class OpType : int {
        Sum = 0,   ///< 求和
        Min = 1,   ///< 最小值
        Max = 2    ///< 最大值
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalQueries         = 0;   ///< 累计查询次数
        quint64 totalUpdates         = 0;   ///< 累计更新次数
        double  avgProcessingTimeMs  = 0.0; ///< 平均处理时间(ms)
        quint64 totalNodesVisited    = 0;   ///< 累计访问节点数
    };

    /**
     * @brief 构造函数
     * @param n 数组大小
     * @param opType 操作类型(Sum/Min/Max)
     * @param parent 父对象
     */
    explicit SegmentTreeLazy(int n = 1, OpType opType = OpType::Sum,
                             QObject* parent = nullptr);

    /**
     * @brief 从数组构建线段树
     * @param data 初始数组
     */
    void build(const QVector<qint64>& data);

    /**
     * @brief 区间加: [l, r] 每个元素加上val
     * @param l 左边界(0-based, inclusive)
     * @param r 右边界(0-based, inclusive)
     * @param val 增量
     */
    void rangeAdd(int l, int r, qint64 val);

    /**
     * @brief 区间赋值: [l, r] 每个元素设为val
     * @param l 左边界
     * @param r 右边界
     * @param val 目标值
     */
    void rangeSet(int l, int r, qint64 val);

    /**
     * @brief 区间查询: [l, r] 的聚合值
     * @param l 左边界
     * @param r 右边界
     * @return 聚合结果(和/最小值/最大值)
     */
    qint64 rangeQuery(int l, int r);

    /**
     * @brief 单点查询
     * @param idx 索引
     * @return 当前值
     */
    qint64 pointQuery(int idx);

    /**
     * @brief 单点更新
     * @param idx 索引
     * @param val 增量
     */
    void pointAdd(int idx, qint64 val);

    /**
     * @brief 获取数组大小
     * @return 大小
     */
    int size() const { return m_n; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 区间更新完成 @param l 左边界 @param r 右边界 @param val 值 */
    void rangeUpdated(int l, int r, qint64 val);

    /** @brief 区间查询完成 @param l 左边界 @param r 右边界 @param result 结果 */
    void rangeQueried(int l, int r, qint64 result);

private:
    /**
     * @brief 下推懒标记
     * @param idx 节点索引
     * @param l 区间左
     * @param r 区间右
     */
    void pushDown(int idx, int l, int r);

    /**
     * @brief 上推合并子节点
     * @param idx 节点索引
     */
    void pushUp(int idx);

    /**
     * @brief 内部区间加
     * @param idx 节点
     * @param l 当前左
     * @param r 当前右
     * @param ql 查询左
     * @param qr 查询右
     * @param val 增量
     */
    void updateAdd(int idx, int l, int r, int ql, int qr, qint64 val);

    /**
     * @brief 内部区间赋值
     */
    void updateSet(int idx, int l, int r, int ql, int qr, qint64 val);

    /**
     * @brief 内部区间查询
     */
    qint64 query(int idx, int l, int r, int ql, int qr);

    /**
     * @brief 内部构建
     */
    void buildInternal(int idx, int l, int r, const std::vector<qint64>& data);

    /** @brief 合并两个值 */
    qint64 merge(qint64 a, qint64 b) const;

    int     m_n;          ///< 数组大小
    OpType  m_opType;     ///< 操作类型

    std::vector<qint64> m_tree;   ///< 线段树数组
    std::vector<qint64> m_lazyAdd;///< 懒标记(加)
    std::vector<bool>   m_hasSet; ///< 是否有赋值标记
    std::vector<qint64> m_lazySet;///< 懒标记(赋值)

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;
};
