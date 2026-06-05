#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AVL平衡二叉搜索树
 *
 * 严格高度平衡的BST，保证O(log n)最坏情况操作。
 */
class AVLTree8 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalInsertions = 0;
        int totalRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AVLTree8(QObject* parent = nullptr);

    /** @brief 插入键值对 */
    void insert(int key, const QVariant& value);

    /** @brief 删除指定键 */
    bool remove(int key);

    /** @brief 获取树高度 */
    int height() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void rotationPerformed(int key, const QString& type);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct AVLNode* m_root = nullptr;
};
