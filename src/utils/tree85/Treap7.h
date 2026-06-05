#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Treap树(树堆)
 *
 * 结合BST和堆性质的随机化平衡树，期望O(log n)操作。
 */
class Treap7 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalInsertions = 0;
        int totalRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Treap7(QObject* parent = nullptr);

    /** @brief 插入键值对 */
    void insert(int key, const QVariant& value);

    /** @brief 按键分裂为两棵子树 */
    QPair<void*, void*> split(int key) const;

    /** @brief 返回所有键的有序列表 */
    QVector<int> inOrderKeys() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void rotationPerformed(int key, const QString& direction);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct TreapNode* m_root = nullptr;
};
