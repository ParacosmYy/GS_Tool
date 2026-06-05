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

    explicit TreapMap(QObject* parent = nullptr);  ///< @brief 构造函数
    ~TreapMap();                                    ///< @brief 析构函数 — 释放所有节点

    TreapMap(const TreapMap&) = delete;             ///< 禁止拷贝
    TreapMap& operator=(const TreapMap&) = delete;

    void insert(int key, double value);     ///< @brief 插入键值对
    bool remove(int key);                   ///< @brief 删除键 @return 是否成功
    bool find(int key, double& value) const;///< @brief 查找键 @return 是否找到

    /** @brief 按阈值分裂树: key<=threshold在左树 */
    void split(int threshold, TreapMap& left, TreapMap& right);
    /** @brief 合并两棵树(要求left所有key < right所有key) */
    static TreapMap* merge(TreapMap* left, TreapMap* right);

    /** @brief 中序遍历，返回按键排序的键值对列表 */
    QVector<std::pair<int, double>> inorderTraversal() const;
    int size() const { return m_size; }    ///< @brief 节点数
    int height() const;                    ///< @brief 树高度

    /** @brief 获取第k小元素(0-based) */
    bool kthElement(int k, int& key, double& value) const;
    void clear();                          ///< @brief 清空树

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

private:
    /** @brief Treap节点 */
    struct Node {
        int key;                    ///< 键(BST性质)
        double value;               ///< 值
        int priority;               ///< 优先级(堆性质)
        int subtreeSize;            ///< 子树大小(顺序统计)
        Node* left;                 ///< 左子树
        Node* right;                ///< 右子树
        explicit Node(int k, double v, int prio)
            : key(k), value(v), priority(prio), subtreeSize(1)
            , left(nullptr), right(nullptr) {}
    };

    Node* rotateRight(Node* node);  ///< @brief 右旋转
    Node* rotateLeft(Node* node);   ///< @brief 左旋转
    Node* insertImpl(Node* node, int key, double value, int priority); ///< @brief 递归插入
    Node* removeImpl(Node* node, int key);          ///< @brief 递归删除
    void splitImpl(Node* node, int threshold, Node*& left, Node*& right); ///< @brief 分裂实现
    Node* mergeImpl(Node* left, Node* right);       ///< @brief 合并实现
    void updateSize(Node* node);    ///< @brief 更新子树大小
    void inorderImpl(Node* node, QVector<std::pair<int, double>>& result) const; ///< @brief 中序遍历
    int heightImpl(Node* node) const;               ///< @brief 递归求高度
    void destroyTree(Node* node);   ///< @brief 递归销毁
    Node* cloneTree(Node* node) const;///< @brief 深拷贝子树

    Node* m_root;                   ///< 树根
    int m_size;                     ///< 节点数
    mutable Stats m_stats;          ///< 统计信息
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
