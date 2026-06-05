/**
 * @file ScapegoatTree2.h
 * @brief 替罪羊树 — 带重建统计的自平衡二叉搜索树
 *
 * 功能: 实现Scapegoat Tree，一种不需要旋转的自平衡BST。
 *       当某子树不平衡度超过阈值(alpha)时，通过"替罪羊"节点
 *       触发子树完全重建来恢复平衡。提供详细的重建统计。
 *
 * 协作: AvlTree(AVL树) / AaTree(AA树) / OrderedDict(有序字典)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>
#include <QElapsedTimer>
#include <functional>

/**
 * @brief 替罪羊树(带重建统计)
 */
class ScapegoatTree2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        int    totalInserts = 0;          ///< 累计插入次数
        int    totalDeletes = 0;          ///< 累计删除次数
        int    totalRebuilds = 0;         ///< 累计重建次数
        int    totalRebuiltNodes = 0;     ///< 累计重建节点数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 重建事件记录
     */
    struct RebuildEvent {
        double triggerKey = 0.0;   ///< 触发重建的键
        int    subtreeSize = 0;    ///< 被重建子树大小
        int    depth = 0;          ///< 触发时的深度
        bool   wasInsert = true;   ///< 是否由插入触发
    };

    explicit ScapegoatTree2(double alpha = 0.67, QObject* parent = nullptr);

    /**
     * @brief 插入键值对
     * @param key 键(唯一)
     * @param value 值
     * @return 是否插入成功(key不重复时返回true)
     */
    bool insert(double key, double value);

    /**
     * @brief 删除键
     * @param key 待删除的键
     * @return 是否删除成功
     */
    bool remove(double key);

    /**
     * @brief 查找键对应的值
     * @param key 键
     * @param value 输出值
     * @return 是否找到
     */
    bool find(double key, double& value) const;

    /**
     * @brief 范围查询 [lo, hi]
     * @param lo 下界
     * @param hi 上界
     * @return 键值对列表(按键排序)
     */
    QVector<QPair<double, double>> rangeQuery(double lo, double hi) const;

    /**
     * @brief 中序遍历
     * @param visitor 访问者函数 (key, value) -> void
     */
    void inorderTraversal(
        const std::function<void(double, double)>& visitor) const;

    /** @brief 获取节点数 */
    int size() const { return m_size; }
    /** @brief 判断是否为空 */
    bool isEmpty() const { return m_size == 0; }
    /** @brief 获取重建事件历史 */
    const QVector<RebuildEvent>& rebuildHistory() const
        { return m_rebuildHistory; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成 @param key 键 @param rebuilt 是否触发重建 */
    void insertCompleted(double key, bool rebuilt);
    /** @brief 删除完成 @param key 键 */
    void removeCompleted(double key);
    /** @brief 重建触发 @param subtreeSize 子树大小 @param depth 深度 */
    void rebuildTriggered(int subtreeSize, int depth);

private:
    /**
     * @brief 树节点
     */
    struct Node {
        double key;           ///< 键
        double value;         ///< 值
        Node*  left = nullptr;  ///< 左子节点
        Node*  right = nullptr; ///< 右子节点
    };

    /** @brief 计算子树大小 */
    int subtreeSize(Node* node) const;

    /** @brief 判断是否不平衡 */
    bool isUnbalanced(Node* node, int leftSize, int rightSize) const;

    /** @brief 查找替罪羊节点 */
    Node* findScapegoat(Node* root, double key, int& outDepth) const;

    /** @brief 重建子树为完美平衡 */
    Node* rebuildSubtree(Node* root);

    /** @brief 中序收集节点到数组 */
    void collectInorder(Node* node, QVector<Node*>& nodes) const;

    /** @brief 从有序数组构建平衡子树 */
    Node* buildBalanced(const QVector<Node*>& nodes, int start, int end);

    /** @brief 递归释放子树 */
    void freeSubtree(Node* node);

    /** @brief 递归查找 */
    bool findHelper(Node* node, double key, double& value) const;

    /** @brief 递归范围查询 */
    void rangeHelper(Node* node, double lo, double hi,
                     QVector<QPair<double, double>>& result) const;

    Node*  m_root = nullptr; ///< 根节点
    int    m_size = 0;       ///< 当前节点数
    int    m_maxSize = 0;    ///< 上次重建后的最大节点数(用于删除判定)
    double m_alpha;          ///< 不平衡阈值(0.5~1.0)

    QVector<RebuildEvent> m_rebuildHistory; ///< 重建事件历史

    Stats              m_stats;
    double             m_timeSum = 0.0;
    mutable QElapsedTimer m_timer;
};
