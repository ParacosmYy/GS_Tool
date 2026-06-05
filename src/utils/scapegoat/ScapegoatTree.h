/**
 * @file ScapegoatTree.h
 * @brief 替罪羊树 — 自平衡二叉搜索树
 *
 * 功能: 实现替罪羊树(Scapegoat Tree)，基于部分重建的自平衡BST，
 *       支持插入、删除、查询、中序遍历，统计插入/重建次数与平均耗时。
 *
 * 协作: StateTracker(状态管理) / DataAggregator(有序聚合)
 */
#ifndef SCAPEGOATTREE_H
#define SCAPEGOATTREE_H

#include <QObject>
#include <QVector>

/**
 * @brief 替罪羊树 — 自平衡二叉搜索树
 */
class ScapegoatTree : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;           ///< 累计插入次数
        quint64 totalRebuilds = 0;          ///< 累计重建次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit ScapegoatTree(QObject* parent = nullptr);
    ~ScapegoatTree();

    /** @brief 插入键 @param key 键值 */
    void insert(double key);

    /** @brief 删除键 @param key 键值 */
    void remove(double key);

    /** @brief 查询键是否存在 @param key 键值 @return 是否存在 */
    bool contains(double key) const;

    /** @brief 中序遍历 @return 有序键列表 */
    QVector<double> inOrder() const;

    /** @brief 树中元素数 @return 元素个数 */
    int size() const { return m_size; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 子树重建信号 @param subtreeSize 重建子树大小 */
    void rebuilt(int subtreeSize);

private:
    /** @brief 树节点 */
    struct Node {
        double key;         ///< 键值
        Node*   left;       ///< 左子树
        Node*   right;      ///< 右子树
        int     subSize;    ///< 子树大小

        explicit Node(double k) : key(k), left(nullptr), right(nullptr), subSize(1) {}
    };

    /** @brief 计算子树大小 @param node 节点 */
    int calcSize(Node* node) const;

    /** @brief 查找替罪羊节点并重建 @param node 当前节点 @param key 插入键 */
    Node* insertAndBalance(Node* node, double key, bool& rebuilt);

    /** @brief 删除节点 @param node 当前节点 @param key 要删除的键 */
    Node* removeNode(Node* node, double key);

    /** @brief 找到最小节点 @param node 根节点 */
    Node* findMin(Node* node) const;

    /** @brief 重建子树 @param node 根节点 @return 新根 */
    Node* rebuildSubtree(Node* node);

    /** @brief 中序收集 @param node 节点 @param result 结果列表 */
    void inOrderCollect(Node* node, QVector<double>& result) const;

    /** @brief 从有序数组构建平衡树 @param sorted 有序数组 @param l 左边界 @param r 右边界 */
    Node* buildBalanced(const QVector<Node*>& sorted, int l, int r);

    /** @brief 扁平化子树 @param node 节点 @param sorted 输出数组 */
    void flatten(Node* node, QVector<Node*>& sorted);

    /** @brief 递归删除 @param node 节点 */
    void destroyTree(Node* node);

    /** @brief 检查是否alpha不平衡 @param node 节点 @return 是否不平衡 */
    bool isUnbalanced(Node* node) const;

    Node*   m_root;         ///< 根节点
    int     m_size;         ///< 元素总数
    double  m_alpha;        ///< 平衡因子(0.5~1.0)
    double  m_timeSum;      ///< 累计耗时(ms)
    mutable Stats m_stats;  ///< 可变统计
};

#endif // SCAPEGOATTREE_H
