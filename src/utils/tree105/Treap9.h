#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Treap树(树堆)实现
 *
 * 结合二叉搜索树和堆性质的概率平衡数据结构，
 * 通过随机优先级实现期望O(logN)的操作复杂度。
 */
class Treap9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit Treap9(QObject* parent = nullptr);

    /** @brief 插入键值对 */
    void insert(double key, int value);

    /** @brief 删除指定键 */
    void remove(double key);

    /** @brief 查找指定键，返回是否找到 */
    bool find(double key);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void inserted(double key);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
