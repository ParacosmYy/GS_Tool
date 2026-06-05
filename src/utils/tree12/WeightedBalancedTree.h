/**
 * @file WeightedBalancedTree.h
 * @brief 加权平衡树(BB[α]) — 动态维护节点权重的平衡二叉搜索树
 *
 * 功能: 实现BB[alpha]加权平衡树，每个节点维护子树权重。插入/删除后
 *       通过全局重建策略恢复平衡。支持排名查询、范围查询和权重统计。
 *
 * 协作: DataAggregator(聚合) / StateTracker(状态追踪)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 加权平衡树(BB[alpha])引擎
 */
class WeightedBalancedTree : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        int totalInsertions = 0;            ///< 累计插入次数
        int totalDeletions = 0;             ///< 累计删除次数
        int totalSearches = 0;              ///< 累计搜索次数
        int totalRebalances = 0;            ///< 累计再平衡次数
        double avgProcessingTimeMs = 0.0;   ///< 平均操作时间(ms)
    };

    explicit WeightedBalancedTree(QObject* parent = nullptr);

    /** @brief 设置平衡因子alpha @param alpha 平衡因子(0.5~1.0，默认0.7) */
    void setAlpha(double alpha);

    /** @brief 插入键值对 @param key 键 @param weight 权重(默认1.0) */
    void insert(double key, double weight = 1.0);

    /** @brief 删除键 @param key 键 @return 是否删除成功 */
    bool remove(double key);

    /** @brief 搜索键 @param key 键 @return (是否存在, 权重) */
    QPair<bool, double> search(double key) const;

    /** @brief 查询第k小的键(按权重排名) @param rank 排名(1-based) @return 键值(不存在返回NaN) */
    double selectByRank(int rank) const;

    /** @brief 查询键的排名 @param key 键 @return 排名(1-based，不存在返回-1) */
    int rankOf(double key) const;

    /** @brief 范围查询: [lo, hi]内的所有键 @param lo 下界 @param hi 上界 @return (键, 权重)列表 */
    QList<QPair<double, double>> rangeQuery(double lo, double hi) const;

    /** @brief 前驱: 小于key的最大键 @param key 键 @return 前驱键(无则NaN) */
    double predecessor(double key) const;

    /** @brief 后继: 大于key的最小键 @param key 键 @return 后继键(无则NaN) */
    double successor(double key) const;

    /** @brief 树中元素总数 @return 元素数 */
    int size() const;

    /** @brief 总权重 @return 权重和 */
    double totalWeight() const;

    /** @brief 树高度 @return 高度(空树返回-1) */
    int height() const;

    /** @brief 检查树是否平衡 @return 是否满足BB[alpha]条件 */
    bool isBalanced() const;

    /** @brief 中序遍历 @return (键, 权重)列表 */
    QList<QPair<double, double>> inOrderTraversal() const;

    /** @brief 清空树 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 再平衡触发 @param nodeCount 节点数 @param triggerKey 触发键 */
    void rebalanced(int nodeCount, double triggerKey);

    /** @brief 插入完成 @param key 插入的键 @param treeSize 树大小 */
    void inserted(double key, int treeSize);

private:
    /** @brief 树节点 */
    struct Node {
        double key = 0.0;           ///< 键
        double weight = 1.0;        ///< 节点权重
        double subWeight = 1.0;    ///< 子树权重和
        int subCount = 1;           ///< 子树节点数
        int height = 1;             ///< 子树高度
        Node* left = nullptr;       ///< 左子树
        Node* right = nullptr;      ///< 右子树
        Node* parent = nullptr;     ///< 父节点
    };

    /** @brief 递归插入 @param node 当前节点 @param key 键 @param weight 权重 @return 新根 */
    Node* insertNode(Node* node, double key, double weight);

    /** @brief 递归删除 @param node 当前节点 @param key 键 @return 新根 */
    Node* deleteNode(Node* node, double key);

    /** @brief 查找节点 @param key 键 @return 节点指针(无则nullptr) */
    Node* findNode(double key) const;

    /** @brief 全局重建子树 @param node 根节点 @return 平衡后的根 */
    Node* rebuildSubtree(Node* node);

    /** @brief 中序收集节点用于重建 @param node 当前节点 @param nodes 输出数组 */
    void collectInOrder(Node* node, QVector<Node*>& nodes);

    /** @brief 从有序数组构建平衡树 @param nodes 节点数组 @param lo 下界 @param hi 上界 @return 根 */
    Node* buildBalanced(QVector<Node*>& nodes, int lo, int hi);

    /** @brief 更新节点聚合信息 @param node 节点 */
    void updateNode(Node* node);

    /** @brief 检查BB[alpha]平衡条件 @param node 节点 @return 是否平衡 */
    bool checkBalance(Node* node) const;

    /** @brief 递归释放 @param node 节点 */
    void freeTree(Node* node);

    /** @brief 递归计算高度 @param node 节点 @return 高度 */
    int treeHeight(Node* node) const;

    Node* m_root = nullptr;                 ///< 树根
    double m_alpha = 0.7;                   ///< 平衡因子
    int m_count = 0;                        ///< 节点计数

    mutable Stats m_stats;                          ///< 运行统计
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
