#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 伸展树(Splay Tree)实现
 *
 * 自调整二叉搜索树,每次访问操作将目标节点旋转至根,
 * 具有优秀的局部性表现,适用于缓存与频率敏感的查找场景。
 */
class SplayTree8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit SplayTree8(QObject* parent = nullptr);

    /** @brief 插入键值对,附带关联数据 */
    void insert(double key, int value);

    /** @brief 移除指定键 */
    void remove(double key);

    /** @brief 查找指定键是否存在 */
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
