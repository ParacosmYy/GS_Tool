#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Fenwick树（树状数组）实现 (8种操作)
 *
 * 提供O(log n)的前缀和查询和单点更新，支持区间查询和区间修改。
 */
class FenwickTree8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalOperations = 0;        ///< 总操作次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int treeSize = 0;               ///< 树大小
    };

    explicit FenwickTree8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 初始化Fenwick树
     * @param initialValues 初始值数组
     */
    void initialize(const QVector<double>& initialValues);

    /**
     * @brief 单点更新（增加增量）
     * @param index 目标索引（0-based）
     * @param delta 增量值
     */
    void update(int index, double delta);

    /**
     * @brief 前缀和查询 [0, index]
     * @param index 查询右端点
     * @return 前缀和
     */
    double prefixSum(int index) const;

    /**
     * @brief 区间和查询 [left, right]
     * @param left 左端点
     * @param right 右端点
     * @return 区间和
     */
    double rangeSum(int left, int right) const;

    /**
     * @brief 获取原始数组值
     * @param index 目标索引
     * @return 当前值
     */
    double valueAt(int index) const;

signals:
    /// 操作完成信号
    void operationCompleted(const QString& operation);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_tree;
    QVector<double> m_original;
    int m_size = 0;
};
