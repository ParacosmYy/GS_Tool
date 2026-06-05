#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 树状数组(Binary Indexed Tree)
 *
 * 支持单点更新和区间求和的高效数据结构，
 * 时间复杂度均为O(log n)，用于频率统计和前缀和查询。
 */
class FenwickTree7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalUpdates = 0;        ///< 更新操作总数
        int totalQueries = 0;        ///< 查询操作总数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit FenwickTree7(QObject* parent = nullptr);

    /** @brief 设置数组大小 */
    void setSize(int n);
    /** @brief 单点更新: 位置idx增加delta */
    void update(int idx, double delta);
    /** @brief 区间查询: [left, right]的和 */
    double query(int left, int right);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 更新完成，返回位置和增量 */
    void updated(int idx, double delta);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_tree;
};
