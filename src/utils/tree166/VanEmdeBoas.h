/**
 * @file VanEmdeBoas.h
 * @brief van Emde Boas树(O(log log U)操作) — van Emde Boas Tree with O(log log U) Insert/Delete/Successor/Predecessor
 *
 * 功能: 实现van Emde Boas (vEB)树数据结构，支持O(log log U)时间复杂度的
 *       插入、删除、后继、前驱和查找操作。U为全域大小。
 *
 * 协作: FibonacciHeap(优先队列) / RedBlackTree(平衡树) / BPlusTree(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief van Emde Boas树
 */
class VanEmdeBoas : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalQueries = 0;          ///< 累计查询次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        int currentSize = 0;               ///< 当前元素数
    };

    explicit VanEmdeBoas(QObject* parent = nullptr);
    ~VanEmdeBoas() override;

    /** @brief 设置全域大小(必须为2的幂) */
    void setUniverseSize(int U);

    /** @brief 插入元素 */
    bool insert(int x);

    /** @brief 删除元素 */
    bool remove(int x);

    /** @brief 查找元素是否存在 */
    bool contains(int x) const;

    /** @brief 查找后继(大于x的最小元素)，-1表示不存在 */
    int successor(int x) const;

    /** @brief 查找前驱(小于x的最大元素)，-1表示不存在 */
    int predecessor(int x) const;

    /** @brief 获取最小元素，-1表示空 */
    int minimum() const;

    /** @brief 获取最大元素，-1表示空 */
    int maximum() const;

    bool isEmpty() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(int x, int size);
    void deleteCompleted(int x);

private:
    /** @brief vEB子树节点 */
    struct VEBNode {
        int universeSize;          ///< 子域大小
        int min;                   ///< 子树最小值(不存储在cluster中)
        int max;                   ///< 子树最大值
        VEBNode* summary;         ///< 摘要结构
        QVector<VEBNode*> cluster;///< 簇数组

        VEBNode(int u);
        ~VEBNode();
    };

    /** @brief 递归插入 */
    void insertRec(VEBNode* node, int x);

    /** @brief 递归删除 */
    void removeRec(VEBNode* node, int x);

    /** @brief 递归查找后继 */
    int successorRec(VEBNode* node, int x) const;

    /** @brief 递归查找前驱 */
    int predecessorRec(VEBNode* node, int x) const;

    /** @brief 递归查找 */
    bool containsRec(VEBNode* node, int x) const;

    /** @brief 高位索引(cluster号) */
    int high(VEBNode* node, int x) const;

    /** @brief 低位索引(簇内偏移) */
    int low(VEBNode* node, int x) const;

    /** @brief 从高低位还原索引 */
    int index(VEBNode* node, int h, int l) const;

    /** @brief 子域大小 */
    int lowerSqrt(int u) const;

    /** @brief 递归清除 */
    void clearRec(VEBNode* node);

    VEBNode* m_root = nullptr;
    int m_universeSize = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
