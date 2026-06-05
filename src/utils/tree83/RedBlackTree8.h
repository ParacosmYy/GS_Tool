#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 红黑树平衡二叉搜索树
 *
 * 支持O(log n)插入/删除/查找的自平衡BST实现。
 */
class RedBlackTree8 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalInsertions = 0;
        int totalDeletions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RedBlackTree8(QObject* parent = nullptr);

    /** @brief 插入键值对 */
    void insert(int key, const QVariant& value);

    /** @brief 删除指定键 */
    bool remove(int key);

    /** @brief 中序遍历返回所有键值对 */
    QVector<QPair<int, QVariant>> inOrderTraversal() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeRotated(int nodeKey, const QString& rotationType);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct Node* m_root = nullptr;
};
