/**
 * @file ScapegoatTree4.h
 * @brief 替罪羊树(重量平衡重建+摊还O(log n)操作) — Scapegoat Tree with Weight-Balanced Rebuilding and Amortized O(log n) Operations
 *
 * 功能: 实现替罪羊树(Scapegoat Tree)，一种无需旋转的自平衡二叉搜索树，
 *       通过重量平衡因子检测不平衡节点并触发子树重建，摊还O(log n)操作。
 *
 * 协作: VanEmdeBoas(vEB树) / RedBlackTree(红黑树) / AVLTree(AVL树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 替罪羊树
 */
class ScapegoatTree4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalRebuilds = 0;         ///< 累计重建次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        int currentSize = 0;               ///< 当前元素数
        int treeHeight = 0;                ///< 当前树高
    };

    explicit ScapegoatTree4(QObject* parent = nullptr);
    ~ScapegoatTree4() override;

    /** @brief 设置平衡因子alpha(0.5~1.0)，越小越严格 */
    void setAlpha(double alpha);

    /** @brief 插入元素 */
    bool insert(double key);

    /** @brief 删除元素 */
    bool remove(double key);

    /** @brief 查找元素是否存在 */
    bool contains(double key) const;

    /** @brief 查找key的排名(小于key的元素数+1) */
    int rank(double key) const;

    /** @brief 查找第k小元素 */
    double kth(int k) const;

    bool isEmpty() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteCompleted(double key);
    void rebuildTriggered(int nodeCount);

private:
    /** @brief 树节点 */
    struct Node {
        double key;            ///< 键值
        Node* left = nullptr;  ///< 左子树
        Node* right = nullptr; ///< 右子树
        int size = 1;          ///< 子树大小(含自身)

        explicit Node(double k) : key(k) {}
    };

    /** @brief 递归查找 */
    bool containsRec(Node* node, double key) const;

    /** @brief 插入并返回(新节点, 深度, 是否触发重建) */
    Node* insertRec(Node* node, double key, int depth, bool& rebuilt);

    /** @brief 计算子树大小 */
    static int treeSize(Node* node);

    /** @brief 重建子树为完美平衡 */
    Node* rebuildSubtree(Node* node);

    /** @brief 中序遍历收集节点 */
    void flatten(Node* node, QVector<Node*>& list) const;

    /** @brief 从有序列表构建平衡子树 */
    Node* buildBalanced(const QVector<Node*>& list, int lo, int hi);

    /** @brief 删除节点 */
    Node* removeRec(Node* node, double key, bool& found);

    /** @brief 找到替罪羊节点(第一个不平衡祖先) */
    Node* findScapegoat(Node* node, double key);

    /** @brief 递归计算排名 */
    int rankRec(Node* node, double key) const;

    /** @brief 递归查找第k小 */
    double kthRec(Node* node, int k) const;

    /** @brief 更新子树大小 */
    static void updateSize(Node* node);

    /** @brief 递归清除 */
    void clearRec(Node* node);

    /** @brief 计算树高 */
    static int height(Node* node);

    double m_alpha = 0.667;
    Node* m_root = nullptr;
    int m_maxSize = 0; /* Track max size for delete-triggered rebuild */

    Stats m_stats;
    double m_timeSum = 0.0;
};
