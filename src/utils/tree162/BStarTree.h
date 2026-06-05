/**
 * @file BStarTree.h
 * @brief B*树(2→3/3→2分裂合并) — B*-Tree with 2-to-3 Split and 3-to-2 Merge
 *
 * 功能: 实现B*树，节点最低填充率2/3(高于B树的1/2)。支持2→3分裂
 *       (两个满节点分裂为三个)和3→2合并(三个稀疏节点合并为两个)。
 *       适用于内存索引和有序集合管理。
 *
 * 协作: BPlusTree(B+树变体) / IntervalHeap(双端优先队列)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QList>

/**
 * @brief B*树，2→3分裂/3→2合并变体
 */
class BStarTree : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;           ///< 累计插入次数
        quint64 totalDeletes = 0;           ///< 累计删除次数
        quint64 totalSplits = 0;            ///< 累计分裂次数
        quint64 totalMerges = 0;            ///< 累计合并次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        int nodeCount = 0;                  ///< 当前节点数
        int height = 0;                     ///< 当前树高度
    };

    explicit BStarTree(QObject* parent = nullptr);

    /**
     * @brief 设置节点的阶数(最大子节点数)
     * @param order 阶数，>= 3
     */
    void setOrder(int order);

    /**
     * @brief 插入键值
     * @param key 键
     * @param value 值
     */
    void insert(double key, double value);

    /**
     * @brief 删除键
     * @param key 键
     * @return 是否成功删除
     */
    bool remove(double key);

    /**
     * @brief 查找键
     * @param key 键
     * @param value 输出值
     * @return 是否找到
     */
    bool find(double key, double& value) const;

    /**
     * @brief 范围查询
     * @param low 下界
     * @param high 上界
     * @return 范围内的键值对
     */
    QVector<QPair<double, double>> rangeQuery(double low, double high) const;

    /** @brief 树是否为空 */
    bool isEmpty() const;

    /** @brief 清空树 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成 @param key 键 @param height 树高度 */
    void insertCompleted(double key, int height);
    /** @brief 删除完成 @param key 键 @param found 是否找到 */
    void deleteCompleted(double key, bool found);

private:
    /** @brief B*树节点 */
    struct BNode {
        QVector<double> keys;       ///< 键列表
        QVector<double> values;     ///< 值列表
        QList<BNode*> children;     ///< 子节点指针列表
        BNode* parent = nullptr;    ///< 父节点指针
        bool isLeaf = true;         ///< 是否叶节点
    };

    /** @brief 2→3分裂 */
    void splitTwoToThree(BNode* node);

    /** @brief 3→2合并 */
    void mergeThreeToTwo(BNode* node);

    /** @brief 在节点中查找插入位置 */
    int findPosition(BNode* node, double key) const;

    /** @brief 递归插入 */
    void insertRecursive(BNode* node, double key, double value);

    /** @brief 递归删除 */
    bool removeRecursive(BNode* node, double key);

    /** @brief 范围查询递归 */
    void rangeQueryRecursive(BNode* node, double low, double high,
                              QVector<QPair<double, double>>& result) const;

    /** @brief 更新统计 */
    void updateStats();

    /** @brief 释放节点 */
    void deleteNode(BNode* node);

    int m_order = 5;             ///< 阶数(最大子节点数)
    int m_maxKeys = 4;           ///< 每节点最大键数 = order - 1
    int m_minKeys = 2;           ///< 最小键数 = ceil(2*(order-1)/3)

    BNode* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;
};
