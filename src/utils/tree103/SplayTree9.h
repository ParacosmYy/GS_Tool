#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 伸展树(Splay Tree)实现
 *
 * 自调整二叉搜索树，每次访问操作将目标节点旋转至根，
 * 具有优秀的时间摊还复杂度和局部性自适应能力。
 */
class SplayTree9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalOperations = 0; double avgProcessingTimeMs = 0.0; };

    explicit SplayTree9(QObject* parent = nullptr);

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
