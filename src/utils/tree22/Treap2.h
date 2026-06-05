/**
 * @file Treap2.h
 * @brief 隐式键Treap — 分裂/合并/区间操作/懒传播
 *
 * 功能: 基于隐式键的Treap(笛卡尔树)，支持按大小分裂合并、
 *       区间反转/求和/最值查询、懒传播标记下推。
 *
 * 协作: DataSegmentAnalyzer(段分析) / EventTimeline(事件时间线)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 隐式键Treap — 可分裂合并的平衡二叉搜索树
 */
class Treap2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalOperations = 0;        ///< 累计操作次数
        quint64 totalSplits = 0;            ///< 累计分裂次数
        quint64 totalMerges = 0;            ///< 累计合并次数
        quint64 totalNodesCreated = 0;      ///< 累计创建节点数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        int     maxTreeDepth = 0;           ///< 历史最大树深度
    };

    explicit Treap2(QObject* parent = nullptr);
    ~Treap2();

    /**
     * @brief 在位置pos插入值
     * @param pos 插入位置(0-based)
     * @param value 插入值
     */
    void insert(int pos, double value);

    /**
     * @brief 批量构建Treap
     * @param values 值序列
     */
    void build(const QVector<double>& values);

    /**
     * @brief 删除区间[l, r]
     * @param l 左端点
     * @param r 右端点
     */
    void eraseRange(int l, int r);

    /**
     * @brief 查询区间[l, r]之和
     * @param l 左端点
     * @param r 右端点
     * @return 区间和
     */
    double rangeSum(int l, int r);

    /**
     * @brief 查询区间[l, r]最小值
     * @param l 左端点
     * @param r 右端点
     * @return 最小值
     */
    double rangeMin(int l, int r);

    /**
     * @brief 反转区间[l, r]
     * @param l 左端点
     * @param r 右端点
     */
    void reverseRange(int l, int r);

    /**
     * @brief 按位置分裂Treap
     * @param pos 分裂位置(左半部分包含[0, pos))
     * @return (左树, 右树)根节点指针对
     */
    QPair<void*, void*> split(int pos);

    /**
     * @brief 合并两棵Treap
     * @param left 左树根
     * @param right 右树根
     * @return 合并后的根
     */
    void* merge(void* left, void* right);

    /** @brief 获取元素总数 @return 树大小 */
    int size() const;

    /** @brief 获取中序遍历结果 @return 有序值序列 */
    QVector<double> toVector();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 操作完成 @param opType 操作类型 @param treeSize 当前树大小 */
    void operationComplete(const QString& opType, int treeSize);

    /** @brief 区间操作完成 @param l 左端点 @param r 右端点 @param result 结果值 */
    void rangeOperationComplete(int l, int r, double result);

private:
    struct Node;
    void pushDown(Node* node);
    void pushUp(Node* node);
    void splitInternal2(Node* node, int pos, Node*& outL, Node*& outR);
    int getSize(Node* node) const;
    double getSum(Node* node) const;
    double getMin(Node* node) const;
    int getDepth(Node* node) const;
    void clearTree(Node* node);
    Node* buildRecursive(const QVector<double>& values, int l, int r);
    void inOrderCollect(Node* node, QVector<double>& result);

    Node* m_root;               ///< 树根节点

    Stats m_stats;
    double m_timeSum = 0.0;     ///< 处理时间累加器
};
