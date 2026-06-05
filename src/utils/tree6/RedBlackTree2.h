/**
 * @file RedBlackTree2.h
 * @brief 红黑树(顺序统计) — 支持select/rank的平衡BST
 *
 * 功能: 实现带顺序统计(第k小/排名)的红黑树，每个节点维护
 *       子树大小，支持O(log n)的选择和排名查询。
 *
 * 协作: DataAggregator(聚合) / SlidingWindowStats(滑动窗口)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 红黑树(顺序统计增强) — select/rank支持
 */
class RedBlackTree2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        int totalInserts = 0;               ///< 累计插入次数
        int totalDeletes = 0;               ///< 累计删除次数
        int totalSearches = 0;              ///< 累计查找次数
        int totalRotations = 0;             ///< 累计旋转次数
        double avgProcessingTimeMs = 0.0;   ///< 平均操作耗时(ms)
        int maxHeight = 0;                  ///< 历史最大树高
    };

    /** @brief 节点颜色 */
    enum class Color { Red, Black };

    /** @brief 树节点 */
    struct Node {
        double key = 0.0;           ///< 键值
        QVariant value;             ///< 关联值
        Color color = Color::Red;   ///< 节点颜色
        int size = 1;               ///< 子树大小(含自身)
        Node* left = nullptr;       ///< 左子节点
        Node* right = nullptr;      ///< 右子节点
        Node* parent = nullptr;     ///< 父节点
    };

    explicit RedBlackTree2(QObject* parent = nullptr);
    ~RedBlackTree2();

    /**
     * @brief 插入键值对
     * @param key 键值
     * @param value 关联值
     */
    void insert(double key, const QVariant& value = QVariant());

    /**
     * @brief 删除键
     * @param key 要删除的键值
     * @return 删除成功返回true
     */
    bool remove(double key);

    /**
     * @brief 查找键
     * @param key 键值
     * @return 找到的节点(只读),未找到返回nullptr
     */
    const Node* find(double key) const;

    /**
     * @brief 选择第k小的元素(顺序统计)
     * @param k 排名(从1开始)
     * @return 第k小节点,超出范围返回nullptr
     */
    const Node* select(int k) const;

    /**
     * @brief 查询键的排名
     * @param key 键值
     * @return 排名(从1开始),不存在返回-1
     */
    int rank(double key) const;

    /** @brief 树中元素数量 @return 数量 */
    int size() const;

    /** @brief 树是否为空 @return 空返回true */
    bool isEmpty() const { return m_root == m_nil; }

    /** @brief 树的高度 @return 高度(空树=0) */
    int height() const;

    /** @brief 中序遍历 @return 有序键值列表 */
    QVector<QPair<double, QVariant>> inOrderTraversal() const;

    /** @brief 范围查询 @param lo 下界 @param hi 上界 @return [lo,hi]内的键值 */
    QVector<QPair<double, QVariant>> rangeQuery(double lo, double hi) const;

    /** @brief 清空树 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成 @param key 键值 @param treeSize 树大小 */
    void nodeInserted(double key, int treeSize);

    /** @brief 删除完成 @param key 键值 @param success 是否成功 */
    void nodeRemoved(double key, bool success);

private:
    /** @brief 左旋 @param x 旋转中心 */
    void rotateLeft(Node* x);

    /** @brief 右旋 @param x 旋转中心 */
    void rotateRight(Node* x);

    /** @brief 插入修复 @param z 新插入节点 */
    void insertFixup(Node* z);

    /** @brief 删除修复 @param x 替换节点 */
    void deleteFixup(Node* x);

    /** @brief 用v子树替换u子树 @param u 被替换 @param v 替换者 */
    void transplant(Node* u, Node* v);

    /** @brief 查找子树最小节点 @param x 子树根 @return 最小节点 */
    Node* minimum(Node* x) const;

    /** @brief 更新节点大小 @param x 目标节点 */
    void updateSize(Node* x);

    /** @brief 获取节点大小 @param x 目标节点 @return 子树大小 */
    int nodeSize(Node* x) const;

    /** @brief 递归删除 @param node 当前节点 */
    void destroyTree(Node* node);

    /** @brief 递归中序遍历 @param node 当前 @param result 结果 */
    void inOrderHelper(Node* node,
                       QVector<QPair<double, QVariant>>& result) const;

    /** @brief 递归计算高度 @param node 当前 @return 高度 */
    int heightHelper(Node* node) const;

    Node* m_root;                   ///< 根节点
    Node* m_nil;                    ///< 哨兵节点
    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
