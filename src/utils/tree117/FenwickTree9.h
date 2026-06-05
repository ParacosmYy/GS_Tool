#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief FenwickTree9 - 树状数组（Fenwick树）第9代实现
 *
 * 提供O(log n)的前缀和查询与单点更新，
 * 支持范围查询、范围更新及二维树状数组。
 */
class FenwickTree9 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalQueryOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit FenwickTree9(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 初始化树状数组
     * @param initialValues 初始值序列
     */
    void build(const QVector<long long>& initialValues);

    /**
     * @brief 单点更新（将位置idx加上delta）
     * @param idx 更新位置（0-based）
     * @param delta 增量值
     */
    void update(int idx, long long delta);

    /**
     * @brief 前缀和查询 [0, idx]
     * @param idx 查询终止位置
     * @return 前缀和
     */
    long long prefixSum(int idx) const;

    /**
     * @brief 范围求和查询 [left, right]
     * @param left 左边界
     * @param right 右边界
     * @return 范围和
     */
    long long rangeSum(int left, int right) const;

    /**
     * @brief 获取指定位置的值
     * @param idx 位置索引
     * @return 该位置的值
     */
    long long valueAt(int idx) const;

signals:
    void queryCompleted(int operationCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
