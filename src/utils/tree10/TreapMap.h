/**
 * @file TreapMap.h
 * @brief Treap映射 — 树堆(Tree+Heap)实现有序映射
 *
 * 功能: 实现Treap(树堆)数据结构，结合二叉搜索树和堆的性质，
 *       支持split/merge操作，所有操作期望O(log n)时间复杂度。
 *       适用于需要有序映射和高效区间操作的场景。
 *
 * 协作: IntervalTree(区间查询) / PriorityQueue(优先队列)
 */
#pragma once

#include <QObject>
#include <QVector>

#include <functional>
#include <utility>
#include <vector>

/**
 * @brief Treap映射 — 基于随机优先级的平衡树
 *
 * 每个节点同时存储key(满足BST性质)和priority(满足堆性质)，
 * 通过随机优先级保证树的平衡性，支持高效的split/merge操作。
 */
class TreapMap : public QObject {
    Q_OBJECT

public:
    /** @brief 键值对类型 */
    using KeyType = int;
    using ValueType = double;

    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;          ///< 累计插入次数
        int totalRemoves = 0;          ///< 累计删除次数
        int totalFinds = 0;            ///< 累计查找次数
        int totalSplits = 0;           ///< 累计分裂次数
        int totalMerges = 0;           ///< 累计合并次数
        int totalRotations = 0;        ///< 累计旋转次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit TreapMap(QObject* parent = nullptr);

    /**
     * @brief 析构函数 — 释放所有节点
     */
    ~TreapMap();

    /* 禁止拷贝 */
    TreapMap(const TreapMap&) = delete;
    TreapMap& operator=(const TreapMap&) = delete;

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
     */
    void insert(int key, double value);

    /**
     * @brief 删除键
     * @param key 要删除的键
     * @return true=删除成功
     */
    bool remove(int key);

    /**
     * @brief 查找键对应的值
     * @param key 键
     * @param value 输出值
     * @return true=找到
     */
    bool find(int key, double& value) const;

    /**
     * @brief 按键范围分裂树
     * @param threshold 分裂阈值: key <= threshold在左树
     * @param left 输出左树(小键)
     * @param right 输出右树(大键)
     */
    void split(int threshold, TreapMap& left, TreapMap& right);

    /**
     * @brief 合并两棵树(要求left所有key < right所有key)
     * @param left 左树
     * @param right 右树
     */
    static TreapMap* merge(TreapMap* left, TreapMap* right);

    /**
     * @brief 中序遍历收集所有键值对
     * @return 按键排序的键值对列表
     */
    QVector<std::pair<int, double>> inorderTraversal() const;

    /**
     * @brief 获取树的大小
     * @return 节点数
     */
    int size() const { return m_size; }

    /**
     * @brief 获取树的高度
     * @return 高度
     */
    int height() const;

    /**
     * @brief 获取第k小的元素(顺序统计)
     * @param k 排名(0-based)
     * @param key 输出键
     * @param value 输出值
     * @return true=成功
     */
    bool kthElement(int k, int& key, double& value) const;

    /** @brief 清空树 */
    void clear();

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

private:
    /**
     * @brief Treap节点
     */
    struct Node {
        int key;                ///< 键(BST性质)
        double value;           ///< 值
        int priority;           ///< 优先级(堆性质)
        int subtreeSize;        ///< 子树大小(顺序统计)
        Node* left;             ///< 左子树
        Node* right;            ///< 右子树

        explicit Node(int k, double v, int prio)
            : key(k), value(v), priority(prio), subtreeSize(1)
            , left(nullptr), right(nullptr) {}
    };

    /**
     * @brief 右旋转
     * @param node 旋转根
     * @return 新根
     */
    Node* rotateRight(Node* node);

    /**
     * @brief 左旋转
     * @param node 旋转根
     * @return 新根
     */
    Node* rotateLeft(Node* node);

    /**
     * @brief 递归插入
     * @param node 当前节点
     * @param key 键
     * @param value 值
     * @param priority 优先级
     * @return 新根
     */
    Node* insertImpl(Node* node, int key, double value, int priority);

    /**
     * @brief 递归删除
     * @param node 当前节点
     * @param key 键
     * @return 新根
     */
    Node* removeImpl(Node* node, int key);

    /**
     * @brief 分裂实现
     * @param node 当前节点
     * @param threshold 阈值
     * @param left 输出左树根
     * @param right 输出右树根
     */
    void splitImpl(Node* node, int threshold, Node*& left, Node*& right);

    /**
     * @brief 合并实现
     * @param left 左树根
     * @param right 右树根
     * @return 合并后的根
     */
    Node* mergeImpl(Node* left, Node* right);

    /** @brief 更新子树大小 @param node 节点 */
    void updateSize(Node* node);

    /** @brief 递归中序遍历 @param node 节点 @param result 结果列表 */
    void inorderImpl(Node* node, QVector<std::pair<int, double>>& result) const;

    /** @brief 递归求高度 @param node 节点 @return 高度 */
    int heightImpl(Node* node) const;

    /** @brief 递归删除所有节点 @param node 节点 */
    void destroyTree(Node* node);

    /** @brief 深拷贝子树 @param node 源节点 @return 新节点 */
    Node* cloneTree(Node* node) const;

    Node* m_root;                   ///< 树根
    int m_size;                     ///< 节点数

    mutable Stats m_stats;          ///< 统计信息
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
