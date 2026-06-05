/**
 * @file SplayTree.h
 * @brief Splay树 — 自调整二叉搜索树
 *
 * 功能: 每次访问后将节点旋转到根，具有优秀的局部性。
 *       摊还O(log n)操作。支持分裂和合并操作。
 *
 * 协作: Treap(随机化) / AvlTree(严格平衡) / BPlusTree(磁盘)
 */
#ifndef SPLAYTREE_H
#define SPLAYTREE_H

#include <QObject>
#include <QVector>

/**
 * @brief Splay树 — 自调整BST
 */
class SplayTree : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;       ///< 累计插入次数
        quint64 totalDeletes = 0;       ///< 累计删除次数
        quint64 totalSearches = 0;      ///< 累计查找次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit SplayTree(QObject* parent = nullptr);
    ~SplayTree();

    /** @brief 插入键值对 @param key 键 @param value 值 */
    void insert(double key, double value);

    /** @brief 删除键 @param key 键 @return 是否成功 */
    bool remove(double key);

    /** @brief 查找键(会将节点展开到根) @param key 键 @return 值指针 */
    const double* find(double key);

    /** @brief 中序遍历 @return 有序键值对 */
    QVector<QPair<double, double>> inorder() const;

    /** @brief 节点数 */
    int size() const { return m_size; }

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 操作完成 @param operation 操作名 @param key 键 */
    void operationCompleted(const QString& operation, double key);

private:
    /** @brief Splay节点 */
    struct Node {
        double key;            ///< 键
        double value;          ///< 值
        Node* left = nullptr;  ///< 左子树
        Node* right = nullptr; ///< 右子树
        Node* parent = nullptr;///< 父节点
    };

    /** @brief Splay操作: 将节点旋转到根 */
    void splay(Node* x);

    /** @brief 左旋 */
    void rotateLeft(Node* x);
    /** @brief 右旋 */
    void rotateRight(Node* x);

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

#endif // SPLAYTREE_H
