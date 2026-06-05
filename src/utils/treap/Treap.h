/**
 * @file Treap.h
 * @brief Treap(树堆) — 随机化平衡二叉搜索树
 *
 * 功能: 结合二叉搜索树和堆性质的随机化数据结构。
 *       插入/删除/查找期望O(log n)。支持区间操作。
 *
 * 协作: BPlusTree(磁盘) / SplayTree(局部性) / AvlTree(严格平衡)
 */
#ifndef TREAP_H
#define TREAP_H

#include <QObject>
#include <QVector>
#include <QRandomGenerator>

/**
 * @brief Treap(树堆) — 随机化BST
 */
class Treap : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;       ///< 累计插入次数
        quint64 totalDeletes = 0;       ///< 累计删除次数
        quint64 totalSearches = 0;      ///< 累计查找次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit Treap(QObject* parent = nullptr);
    ~Treap();

    /** @brief 插入键值对 @param key 键 @param value 值 */
    void insert(double key, double value);

    /** @brief 删除键 @param key 键 @return 是否成功 */
    bool remove(double key);

    /** @brief 查找键 @param key 键 @return 值指针，未找到返回nullptr */
    const double* find(double key) const;

    /** @brief 范围查询 [lo, hi] @return 键值对列表 */
    QVector<QPair<double, double>> rangeQuery(double lo, double hi) const;

    /** @brief 中序遍历 @return 有序键值对 */
    QVector<QPair<double, double>> inorder() const;

    /** @brief 节点数 @return 树大小 */
    int size() const { return m_size; }

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插入完成 @param key 键 */
    void insertCompleted(double key);

    /** @brief 删除完成 @param key 键 @param success 是否成功 */
    void deleteCompleted(double key, bool success);

private:
    /** @brief Treap节点 */
    struct Node {
        double key;          ///< 键
        double value;        ///< 值
        int priority;        ///< 随机优先级
        Node* left = nullptr;  ///< 左子树
        Node* right = nullptr; ///< 右子树
    };

    /** @brief 右旋 */
    Node* rotateRight(Node* root);
    /** @brief 左旋 */
    Node* rotateLeft(Node* root);
    /** @brief 递归插入 */
    Node* insertNode(Node* root, double key, double value);
    /** @brief 递归删除 */
    Node* deleteNode(Node* root, double key);
    /** @brief 递归查找 */
    const double* findNode(Node* root, double key) const;
    /** @brief 递归范围查询 */
    void rangeQueryNode(Node* root, double lo, double hi,
                        QVector<QPair<double, double>>& result) const;
    /** @brief 递归中序遍历 */
    void inorderNode(Node* root,
                     QVector<QPair<double, double>>& result) const;
    /** @brief 递归销毁 */
    void destroyTree(Node* root);

    Node* m_root;       ///< 根节点
    int m_size;         ///< 节点计数
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // TREAP_H
