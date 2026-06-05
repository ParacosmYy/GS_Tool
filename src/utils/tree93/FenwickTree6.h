#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 树状数组(Binary Indexed Tree)工具类
 *
 * 提供树状数组数据结构，支持单点更新和区间求和，
 * 时间复杂度均为O(log n)。
 */
class FenwickTree6 : public QObject {
    Q_OBJECT
public:
    /// 操作统计信息
    struct Stats {
        int totalOperations = 0;    ///< 总操作次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit FenwickTree6(QObject* parent = nullptr);

    /** @brief 设置数组大小 */
    void setSize(int size);

    /** @brief 单点更新：将位置index增加delta */
    void update(int index, double delta);

    /** @brief 区间查询：返回[left, right]的和 */
    double query(int left, int right);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 更新完成信号，返回更新位置和值 */
    void updated(int index, double delta);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_size = 0;
    QVector<double> m_tree;
};
