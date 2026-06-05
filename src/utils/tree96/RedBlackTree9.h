#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 红黑树(Red-Black Tree)实现
 *
 * 自平衡二叉搜索树,通过着色规则与旋转操作保证O(log n)最坏情况性能,
 * 适用于关联容器底层实现、内存管理与调度器数据结构。
 */
class RedBlackTree9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit RedBlackTree9(QObject* parent = nullptr);

    /** @brief 插入键值对 */
    void insert(double key, int value);

    /** @brief 移除指定键 */
    void remove(double key);

    /** @brief 查询键是否存在 */
    void contains(double key);

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
