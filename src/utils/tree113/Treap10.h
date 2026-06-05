#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Treap(树堆)实现
 *
 * 结合二叉搜索树和堆性质的概率平衡树结构，每个节点持有随机优先级，
 * 通过旋转维持堆性质，期望O(log n)操作复杂度，实现简洁且不易退化。
 */
class Treap10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit Treap10(QObject* parent = nullptr);

    /** @brief 插入键值对，随机优先级自动维持平衡 */
    void insert(int key, const QVariant& value);

    /** @brief 查找指定键并返回关联值 */
    QVariant search(int key) const;

    /** @brief 删除指定键并保持Treap性质 */
    bool remove(int key);

    /** @brief 按序分裂为两棵Treap：键<=splitKey和键>splitKey */
    QPair<QVector<QPair<int, QVariant>>, QVector<QPair<int, QVariant>>> split(int splitKey);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 操作完成信号，返回操作类型和当前树大小 */
    void operationCompleted(const QString& operationType, int treeSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
