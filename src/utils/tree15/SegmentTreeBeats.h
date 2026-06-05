/**
 * @file SegmentTreeBeats.h
 * @brief Segment Tree Beats — 区间最小/最大值赋值查询
 *
 * 功能: 实现 Segment Tree Beats 数据结构，支持区间 chmin (取最小)、
 *       chmax (取最大)、区间加法、区间赋值、区间求和、区间最值查询。
 *       所有操作均摊 O(log^2 n) 时间复杂度。
 *
 * 协作: DataBatchProcessor(批量处理) / SlidingWindowStats(滑动窗口)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief Segment Tree Beats — 高级线段树
 *
 * 基于 Segment Tree Beats 算法，支持以下操作:
 *   - rangeChmin(l, r, x): 对 [l,r] 区间每个值执行 min(a_i, x)
 *   - rangeChmax(l, r, x): 对 [l,r] 区间每个值执行 max(a_i, x)
 *   - rangeAdd(l, r, x): 对 [l,r] 区间每个值加 x
 *   - rangeAssign(l, r, x): 将 [l,r] 区间所有值设为 x
 *   - rangeSum(l, r): 查询区间和
 *   - rangeMax(l, r): 查询区间最大值
 *   - rangeMin(l, r): 查询区间最小值
 *
 * 每个操作摊还 O(log^2 n)，空间 O(n)。
 */
class SegmentTreeBeats : public QObject {
    Q_OBJECT

public:
    /** @brief 查询结果 */
    struct RangeResult {
        double sum = 0.0;       ///< 区间和
        double maximum = 0.0;   ///< 区间最大值
        double minimum = 0.0;   ///< 区间最小值
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalUpdates = 0;              ///< 累计更新操作次数
        int totalQueries = 0;              ///< 累计查询次数
        int totalNodesTouched = 0;         ///< 累计访问节点数
        int totalBeatPrunes = 0;           ///< 累计 Beats 剪枝次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit SegmentTreeBeats(QObject* parent = nullptr);

    /**
     * @brief 从数组构建线段树
     * @param data 初始数据
     */
    void build(const QVector<double>& data);

    /**
     * @brief 区间取最小: a_i = min(a_i, x) for i in [l, r]
     * @param l 左端点 (含, 0-based)
     * @param r 右端点 (含)
     * @param x 上限值
     */
    void rangeChmin(int l, int r, double x);

    /**
     * @brief 区间取最大: a_i = max(a_i, x) for i in [l, r]
     * @param l 左端点 (含, 0-based)
     * @param r 右端点 (含)
     * @param x 下限值
     */
    void rangeChmax(int l, int r, double x);

    /**
     * @brief 区间加法: a_i += x for i in [l, r]
     * @param l 左端点
     * @param r 右端点
     * @param x 增量
     */
    void rangeAdd(int l, int r, double x);

    /**
     * @brief 区间赋值: a_i = x for i in [l, r]
     * @param l 左端点
     * @param r 右端点
     * @param x 目标值
     */
    void rangeAssign(int l, int r, double x);

    /**
     * @brief 区间查询
     * @param l 左端点
     * @param r 右端点
     * @return 和/最大/最小
     */
    RangeResult rangeQuery(int l, int r);

    /**
     * @brief 获取单个元素的值
     * @param index 索引
     * @return 元素值
     */
    double pointQuery(int index);

    /** @brief 获取数据大小 */
    int size() const { return m_size; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 更新完成 @param l 左端点 @param r 右端点 @param op 操作类型 */
    void updated(int l, int r, const QString& op);

    /** @brief 查询完成 @param l 左端点 @param r 右端点 */
    void queried(int l, int r);

private:
    struct Node {
        double maxVal = 0.0;      ///< 区间最大值
        double secondMax = 0.0;   ///< 次大值
        int maxCount = 0;         ///< 最大值个数
        double minVal = 0.0;      ///< 区间最小值
        double secondMin = 0.0;   ///< 次小值
        int minCount = 0;         ///< 最小值个数
        double sum = 0.0;         ///< 区间和
        double lazyAdd = 0.0;     ///< 加法懒标记
        double lazyAssign = 0.0;  ///< 赋值懒标记 (NaN 表示无标记)
        bool hasAssign = false;   ///< 是否有赋值懒标记
        int len = 0;              ///< 区间长度
    };

    void buildImpl(int node, int l, int r, const QVector<double>& data);
    void pushDown(int node);
    void pushUp(int node);
    void updateChmin(int node, int l, int r, int ql, int qr, double x);
    void updateChmax(int node, int l, int r, int ql, int qr, double x);
    void updateAdd(int node, int l, int r, int ql, int qr, double x);
    void updateAssign(int node, int l, int r, int ql, int qr, double x);
    RangeResult queryImpl(int node, int l, int r, int ql, int qr);
    void applyAdd(int node, double x);
    void applyAssign(int node, double x);
    void applyChmin(int node, double x);
    void applyChmax(int node, double x);

    QVector<Node> m_tree;         ///< 线段树节点数组
    int m_size;                   ///< 数据规模
    int m_nodeCount;              ///< 节点总数

    Stats m_stats;                ///< 统计信息
    double m_timeSum = 0.0;       ///< 处理时间累加器
};
