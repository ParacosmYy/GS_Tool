/**
 * @file LazySegmentTree.h
 * @brief 懒传播线段树 — 区间更新/区间查询
 *
 * 功能: 实现带懒标记传播的线段树，支持O(log n)的区间加法、
 *       区间赋值、区间最值/求和查询，适用于数据流的窗口统计。
 *
 * 协作: SlidingWindowStats(滑动窗口) / DataThresholdMonitor(阈值)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <limits>

/**
 * @brief 懒传播线段树 — 区间更新与查询
 */
class LazySegmentTree : public QObject {
    Q_OBJECT

public:
    /** @brief 更新操作类型 */
    enum class UpdateOp {
        Add,            ///< 区间加法
        Assign,         ///< 区间赋值
        Multiply        ///< 区间乘法
    };
    Q_ENUM(UpdateOp)

    /** @brief 查询类型 */
    enum class QueryType {
        Sum,            ///< 区间求和
        Min,            ///< 区间最小值
        Max,            ///< 区间最大值
        All             ///< 同时返回sum/min/max
    };
    Q_ENUM(QueryType)

    /** @brief 统计 */
    struct Stats {
        int totalUpdates = 0;               ///< 累计更新次数
        int totalQueries = 0;               ///< 累计查询次数
        int totalPropagations = 0;          ///< 累计懒标记传播次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        quint64 totalNodesVisited = 0;      ///< 累计访问节点数
    };

    /** @brief 查询结果 */
    struct QueryResult {
        double sum = 0.0;       ///< 区间和
        double min = 0.0;       ///< 区间最小
        double max = 0.0;       ///< 区间最大
        int count = 0;          ///< 区间元素数
    };

    explicit LazySegmentTree(QObject* parent = nullptr);

    /**
     * @brief 用初始数据构建线段树
     * @param data 初始数据
     */
    void build(const QVector<double>& data);

    /**
     * @brief 区间更新
     * @param left 左端点(含)
     * @param right 右端点(含)
     * @param value 更新值
     * @param op 操作类型
     */
    void rangeUpdate(int left, int right, double value,
                     UpdateOp op = UpdateOp::Add);

    /**
     * @brief 区间查询
     * @param left 左端点(含)
     * @param right 右端点(含)
     * @param type 查询类型
     * @return 查询结果
     */
    QueryResult rangeQuery(int left, int right,
                           QueryType type = QueryType::All) const;

    /**
     * @brief 单点更新
     * @param index 位置
     * @param value 新值
     * @param op 操作类型
     */
    void pointUpdate(int index, double value,
                     UpdateOp op = UpdateOp::Assign);

    /**
     * @brief 单点查询
     * @param index 位置
     * @return 该点的值
     */
    double pointQuery(int index) const;

    /** @brief 数据大小 @return 元素数量 */
    int size() const { return m_size; }

    /** @brief 是否已构建 @return 已构建返回true */
    bool isBuilt() const { return m_size > 0; }

    /** @brief 获取原始数据快照 @return 数据 */
    QVector<double> data() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 区间更新完成 @param left 左端 @param right 右端 @param value 更新值 */
    void rangeUpdated(int left, int right, double value);

    /** @brief 区间查询完成 @param left 左端 @param right 右端 @param result 结果 */
    void rangeQueried(int left, int right, QueryResult result);

private:
    /** @brief 线段树节点 */
    struct Node {
        double sum = 0.0;           ///< 区间和
        double minVal = 0.0;        ///< 区间最小
        double maxVal = 0.0;        ///< 区间最大
        double lazyAdd = 0.0;       ///< 待加懒标记
        double lazyAssign = 0.0;    ///< 待赋值懒标记
        bool hasLazyAssign = false; ///< 是否有赋值标记
        double lazyMul = 1.0;       ///< 待乘懒标记
        bool hasLazyMul = false;    ///< 是否有乘标记
    };

    /** @brief 向上更新 @param idx 节点索引 */
    void pushUp(int idx);

    /** @brief 向下传播懒标记 @param idx 节点索引 @param l 左界 @param r 右界 */
    void pushDown(int idx, int l, int r) const;

    /** @brief 递归构建 @param idx 节点 @param l 左界 @param r 右界 @param data 数据 */
    void buildHelper(int idx, int l, int r, const QVector<double>& data);

    /** @brief 递归更新 @param idx 节点 @param l 左界 @param r 右界 @param ql 查询左 @param qr 查询右 @param val 值 @param op 操作 */
    void updateHelper(int idx, int l, int r, int ql, int qr,
                      double val, UpdateOp op) const;

    /** @brief 递归查询 @param idx 节点 @param l 左界 @param r 右界 @param ql 查询左 @param qr 查询右 @param result 结果 */
    void queryHelper(int idx, int l, int r, int ql, int qr,
                     QueryResult& result) const;

    int m_size = 0;                         ///< 数据大小
    mutable QVector<Node> m_tree;           ///< 线段树节点数组(mutable for lazy)
    Stats m_stats;                          ///< 统计信息
    double m_timeSum = 0.0;                 ///< 累计耗时
};
