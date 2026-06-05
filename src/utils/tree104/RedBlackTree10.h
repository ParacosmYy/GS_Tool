#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 红黑树(Red-Black Tree)实现
 *
 * 自平衡二叉搜索树，通过节点着色和旋转规则保证O(logN)操作复杂度，
 * 广泛用于关联容器和内核数据结构。
 */
class RedBlackTree10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit RedBlackTree10(QObject* parent = nullptr);

    /** @brief 插入键值对 */
    void insert(double key, int value);

    /** @brief 删除指定键 */
    void remove(double key);

    /** @brief 判断是否包含指定键 */
    bool contains(double key);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void inserted(double key);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
