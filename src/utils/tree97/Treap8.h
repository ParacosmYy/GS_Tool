#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Treap树(树堆)实现
 *
 * 结合二叉搜索树与堆性质的随机化数据结构,通过随机优先级
 * 保持平衡,实现简洁且期望O(log n)性能,适用于随机化算法与竞赛编程。
 */
class Treap8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit Treap8(QObject* parent = nullptr);

    /** @brief 插入键值对 */
    void insert(double key, int value);

    /** @brief 移除指定键 */
    void remove(double key);

    /** @brief 查找指定键 */
    void find(double key);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void inserted(double key);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
