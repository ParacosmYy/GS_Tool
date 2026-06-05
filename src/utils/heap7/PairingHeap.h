/**
 * @file PairingHeap.h
 * @brief 配对堆 — 支持merge/decrease-key的优先队列
 *
 * 功能: 实现配对堆(Pairing Heap)，一种简单高效的合并堆，
 *       支持insert/merge/findMin/deleteMin/decrease-key操作，
 *       均摊时间复杂度接近斐波那契堆。
 *
 * 协作: DataTrigger(触发器) / ShortestPath(最短路)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QSharedPointer>
#include <functional>

/**
 * @brief 配对堆 — 高效合并优先队列
 */
class PairingHeap : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        int totalInserts = 0;              ///< 累计插入次数
        int totalDeleteMins = 0;           ///< 累计删除最小次数
        int totalMerges = 0;               ///< 累计合并次数
        int totalDecreaseKeys = 0;         ///< 累计减小键值次数
        double avgProcessingTimeMs = 0.0;  ///< 平均操作耗时(ms)
        int totalNodesCreated = 0;         ///< 累计创建节点数
    };

    /** @brief 堆节点 */
    struct Node {
        double key = 0.0;                          ///< 键值
        QVariant data;                             ///< 关联数据
        Node* child = nullptr;                     ///< 第一个子节点
        Node* sibling = nullptr;                   ///< 右兄弟节点
        Node* parent = nullptr;                    ///< 父节点(用于decrease-key)
    };

    explicit PairingHeap(QObject* parent = nullptr);
    ~PairingHeap();

    /** @brief 插入元素 @param key 键值 @param data 关联数据 @return 节点指针 */
    Node* insert(double key, const QVariant& data = QVariant());

    /** @brief 合并另一个堆 @param other 被合并的堆(合并后清空) */
    void merge(PairingHeap* other);

    /** @brief 查找最小键值 @return 最小节点指针(空堆返回nullptr) */
    Node* findMin() const;

    /** @brief 删除最小元素 @return 被删除节点的数据 */
    QVariant deleteMin();

    /** @brief 减小节点键值 @param node 目标节点 @param newKey 新键值(必须<当前) */
    void decreaseKey(Node* node, double newKey);

    /** @brief 堆是否为空 @return 空返回true */
    bool isEmpty() const { return m_root == nullptr; }

    /** @brief 获取堆中元素数量 @return 元素数量 */
    int size() const { return m_size; }

    /** @brief 清空堆 */
    void clear();

    /** @brief 将所有元素按键值排序输出 @return 排序后的键值列表 */
    QVector<QPair<double, QVariant>> toSortedList();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最小元素被删除 @param key 被删除的键值 */
    void minDeleted(double key);

    /** @brief 堆被合并 @param otherSize 被合并堆的大小 */
    void heapMerged(int otherSize);

private:
    /** @brief 合并两棵树 @param a 树A @param b 树B @return 合并后的根 */
    Node* mergeTrees(Node* a, Node* b);

    /** @brief 两趟合并(删除最小后子节点合并) @param first 第一个子节点 @return 合并后的根 */
    Node* twoPassMerge(Node* first);

    /** @brief 递归删除子树 @param node 根节点 */
    void destroyTree(Node* node);

    /** @brief 计算子树大小 @param node 根节点 @return 节点数 */
    int countNodes(Node* node) const;

    Node* m_root = nullptr;         ///< 堆根节点
    int m_size = 0;                 ///< 元素数量
    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
