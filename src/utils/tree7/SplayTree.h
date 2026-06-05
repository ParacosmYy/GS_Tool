/**
 * @file SplayTree.h
 * @brief Splay伸展树 — 自调整二叉搜索树
 *
 * 功能:
 *   - Zig单旋转: x为根的直接子节点
 *   - Zig-Zig双旋: x与父节点同侧
 *   - Zig-Zag双旋: x与父节点异侧
 *   - 支持插入、删除、查找、区间查询
 *   - 摊还O(log N)操作复杂度
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class SplayTree
 * @brief Splay伸展树 — 基于自调整策略的平衡BST
 *
 * 每次访问后将节点旋转至根，热点数据自动聚集在顶部。
 * 无需显式平衡信息，适合频繁访问局部数据的场景。
 */
class SplayTree : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;           /**< 总插入次数 */
        int totalDeletes = 0;           /**< 总删除次数 */
        int totalSearches = 0;          /**< 总查找次数 */
        int totalRotations = 0;         /**< 总旋转次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 树节点 */
    struct Node {
        int key;                        /**< 键值 */
        QString value;                  /**< 关联值 */
        Node* left = nullptr;           /**< 左子节点 */
        Node* right = nullptr;          /**< 右子节点 */
        Node* parent = nullptr;         /**< 父节点 */
    };

    /** @brief 构造函数 */
    explicit SplayTree(QObject* parent = nullptr);

    /** @brief 析构函数: 释放所有节点 */
    ~SplayTree();

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
     */
    void insert(int key, const QString& value = QString());

    /**
     * @brief 删除键
     * @param key 要删除的键
     * @return 是否删除成功
     */
    bool remove(int key);

    /**
     * @brief 查找键
     * @param key 目标键
     * @return 关联值(未找到返回空)
     */
    QString search(int key);

    /**
     * @brief 检查键是否存在
     * @param key 目标键
     */
    bool contains(int key);

    /**
     * @brief 中序遍历(有序输出)
     * @return 有序键值对列表
     */
    QVector<QPair<int, QString>> inOrderTraversal() const;

    /**
     * @brief 查找区间 [lo, hi] 内的所有键值对
     * @param lo 下界
     * @param hi 上界
     * @return 范围内的键值对
     */
    QVector<QPair<int, QString>> rangeQuery(int lo, int hi) const;

    /** @brief 树的节点数 */
    int size() const;

    /** @brief 树是否为空 */
    bool isEmpty() const;

    /** @brief 获取树高度 */
    int height() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插入完成 */
    void inserted(int key);

    /** @brief 删除完成 */
    void removed(int key, bool success);

    /** @brief 查找完成 */
    void searched(int key, bool found);

private:
    /** @brief Splay操作: 将节点旋转至根 */
    void splay(Node* x);

    /** @brief 左旋 */
    void rotateLeft(Node* x);

    /** @brief 右旋 */
    void rotateRight(Node* x);

    /** @brief 递归中序遍历 */
    void inOrderHelper(Node* node,
                       QVector<QPair<int, QString>>& result) const;

    /** @brief 递归范围查询 */
    void rangeHelper(Node* node, int lo, int hi,
                     QVector<QPair<int, QString>>& result) const;

    /** @brief 递归计算高度 */
    int heightHelper(Node* node) const;

    /** @brief 递归销毁节点 */
    void destroyTree(Node* node);

    /** @brief 查找子树中的最大节点 */
    Node* findMax(Node* node) const;

    Node* m_root = nullptr;          /**< 根节点 */
    int m_size = 0;                  /**< 节点计数 */
    Stats m_stats;                   /**< 统计信息 */
    double m_timeSum = 0.0;          /**< 累计时间 */
};
