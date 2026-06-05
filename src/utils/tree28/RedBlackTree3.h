/**
 * @file RedBlackTree3.h
 * @brief 红黑树(顺序统计增强) — 攒序选择+排名查询
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 顺序统计增强红黑树
 * 支持按秩选择、排名查询、区间统计
 */
class RedBlackTree3 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInsertions = 0;
        int totalDeletions = 0;
        int totalQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RedBlackTree3(QObject* parent = nullptr);
    ~RedBlackTree3();

    /** @brief 插入键值对 @param key 键 @param value 值 */
    void insert(double key, int value);

    /** @brief 删除键 @return 是否成功 */
    bool remove(double key);

    /** @brief 查找键 @return 值(-1=不存在) */
    int find(double key) const;

    /** @brief 按秩选择: 第k小元素的键 @param k 排名(1-based) @return 键 */
    double selectKth(int k) const;

    /** @brief 排名查询: key的排名(1-based) @return 排名(-1=不存在) */
    int rank(double key) const;

    /** @brief 范围计数: [low,high]间元素数 */
    int rangeCount(double low, double high) const;

    /** @brief 前驱(key的最大小于key的键) */
    double predecessor(double key) const;

    /** @brief 后继(key的最小大于key的键) */
    double successor(double key) const;

    /** @brief 元素数 */
    int size() const { return m_size; }

    /** @brief 是否为空 */
    bool isEmpty() const { return m_size == 0; }

    /** @brief 清空 */
    void clear();

    /** @brief 有序遍历 @return (键,值)列表 */
    QVector<QPair<double,int>> inOrderTraversal() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void inserted(double key);
    void removed(double key);

private:
    enum Color { Red, Black };
    struct Node {
        double key;
        int value;
        Color color;
        int subtreeSize;
        Node *left, *right, *parent;
    };

    void leftRotate(Node* x);
    void rightRotate(Node* y);
    void insertFixup(Node* z);
    void deleteFixup(Node* x);
    void transplant(Node* u, Node* v);
    Node* treeMinimum(Node* x) const;
    void updateSize(Node* n);
    Node* selectKthNode(Node* x, int k) const;
    int rankOf(Node* x) const;
    void destroyTree(Node* n);

    Node* m_nil;
    Node* m_root;
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
