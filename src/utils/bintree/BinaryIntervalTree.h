/**
 * @file BinaryIntervalTree.h
 * @brief 二叉区间树 — O(log n)范围查询(和/最小/最大)
 *
 * 功能: 基于线段树(Segment Tree)的二叉区间树，支持O(log n)的
 *       单点更新和区间查询(求和/最小值/最大值)，适用于大规模
 *       数据的实时区间统计。
 *
 * 协作: KMeansClusterer(数据分簇) / DataAggregator(数据聚合)
 */
#ifndef BINARYINTERVALTREE_H
#define BINARYINTERVALTREE_H

#include <QObject>
#include <QVector>

/**
 * @brief 二叉区间树 — 线段树区间查询
 */
class BinaryIntervalTree : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalUpdates = 0;         ///< 累计更新次数
        quint64 totalQueries = 0;         ///< 累计查询次数
    };

    explicit BinaryIntervalTree(QObject* parent = nullptr);

    /** @brief 构建区间树
     *  @param values 初始数据 */
    void build(const QVector<double>& values);

    /** @brief 单点更新
     *  @param index 索引
     *  @param value 新值 */
    void update(int index, double value);

    /** @brief 区间求和
     *  @param left  左边界(包含)
     *  @param right 右边界(包含)
     *  @return 区间和 */
    double rangeSum(int left, int right);

    /** @brief 区间最小值
     *  @param left  左边界(包含)
     *  @param right 右边界(包含)
     *  @return 最小值 */
    double rangeMin(int left, int right);

    /** @brief 区间最大值
     *  @param left  左边界(包含)
     *  @param right 右边界(包含)
     *  @return 最大值 */
    double rangeMax(int left, int right);

    /** @brief 数据规模 @return 元素数量 */
    int size() const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 树更新 @param index 更新索引 @param value 新值 */
    void treeUpdated(int index, double value);

private:
    /** @brief 构建求和树递归 @param node 节点 @param left 左边界 @param right 右边界 */
    void buildSum(int node, int left, int right);

    /** @brief 构建最小树递归 @param node 节点 @param left 左边界 @param right 右边界 */
    void buildMin(int node, int left, int right);

    /** @brief 构建最大树递归 @param node 节点 @param left 左边界 @param right 右边界 */
    void buildMax(int node, int left, int right, QVector<double>& maxTree);

    /** @brief 更新求和树 @param node 节点 @param left 左 @param right 右 @param idx 目标索引 @param val 新值 */
    void updateSum(int node, int left, int right, int idx, double val);

    /** @brief 更新最小树 @param node 节点 @param left 左 @param right 右 @param idx 目标索引 @param val 新值 */
    void updateMin(int node, int left, int right, int idx, double val);

    /** @brief 更新最大树 @param tree 树数组 @param node 节点 @param left 左 @param right 右 @param idx 目标索引 @param val 新值 */
    void updateMaxInTree(QVector<double>& tree, int node, int left, int right,
                         int idx, double val);

    /** @brief 查询区间和 @param node 节点 @param left 左 @param right 右 @param ql 查询左 @param qr 查询右 */
    double querySum(int node, int left, int right, int ql, int qr) const;

    /** @brief 查询区间最小 @param node 节点 @param left 左 @param right 右 @param ql 查询左 @param qr 查询右 */
    double queryMin(int node, int left, int right, int ql, int qr) const;

    /** @brief 查询区间最大 @param tree 树数组 @param node 节点 @param left 左 @param right 右 @param ql 查询左 @param qr 查询右 */
    double queryMax(const QVector<double>& tree, int node, int left, int right,
                    int ql, int qr) const;

    QVector<double> m_data;        ///< 原始数据
    QVector<double> m_sumTree;     ///< 求和线段树
    QVector<double> m_minTree;     ///< 最小线段树
    QVector<double> m_maxTree;     ///< 最大线段树
    int m_n;                       ///< 数据规模

    mutable Stats m_stats;         ///< 可变统计
};

#endif // BINARYINTERVALTREE_H
