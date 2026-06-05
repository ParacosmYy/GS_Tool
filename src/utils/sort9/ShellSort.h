/**
 * @file ShellSort.h
 * @brief Shell排序 — Ciura/Tokuda间隙序列的高效变体
 *
 * 功能:
 *   - Ciura间隙序列: 经验最优序列 [701, 301, 132, 57, 23, 10, 4, 1]
 *   - Tokuda间隙序列: 理论推导序列 h_k = ceil(9*(9/4)^(k-1) - 4)
 *   - Hibbard/Sedgewick序列可选
 *   - 支持自定义比较器
 *   - 原地排序, 最坏O(N^1.5)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>
#include <functional>

/**
 * @class ShellSort
 * @brief Shell排序引擎 — 多种间隙序列策略
 *
 * Shell排序通过分组插入排序逐步减小间隙，最终gap=1时即标准插入排序。
 * 间隙序列的选择决定排序效率: Ciura序列实测最优。
 */
class ShellSort : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSorts = 0;            /**< 总排序次数 */
        int totalElements = 0;         /**< 总排序元素数 */
        int totalComparisons = 0;      /**< 总比较次数 */
        int totalSwaps = 0;            /**< 总交换次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 间隙序列类型 */
    enum class GapSequence {
        Ciura,      /**< Ciura经验序列(实测最优) */
        Tokuda,     /**< Tokuda理论序列 */
        Hibbard,    /**< Hibbard序列: 2^k - 1 */
        Sedgewick,  /**< Sedgewick序列 */
        Shell       /**< Shell原始: N/2, N/4, ..., 1 */
    };
    Q_ENUM(GapSequence)

    /** @brief 构造函数 */
    explicit ShellSort(QObject* parent = nullptr);

    /**
     * @brief 对QVector<double>排序
     * @param data 待排序数据
     * @param ascending 升序(默认true)
     * @param seq 间隙序列(默认Ciura)
     * @return 排序后的数据
     */
    QVector<double> sort(const QVector<double>& data,
                           bool ascending = true,
                           GapSequence seq = GapSequence::Ciura) const;

    /**
     * @brief 对QStringList排序
     * @param data 待排序列表
     * @param caseSensitive 大小写敏感
     * @param seq 间隙序列
     * @return 排序后的列表
     */
    QStringList sortStrings(const QStringList& data,
                              bool caseSensitive = true,
                              GapSequence seq = GapSequence::Ciura) const;

    /**
     * @brief 对QVector<int>排序
     * @param data 待排序数据
     * @param ascending 升序
     * @param seq 间隙序列
     * @return 排序后的数据
     */
    QVector<int> sortInt(const QVector<int>& data,
                           bool ascending = true,
                           GapSequence seq = GapSequence::Ciura) const;

    /**
     * @brief 带索引追踪的排序(返回排序后的索引)
     * @param data 待排序数据
     * @param ascending 升序
     * @param seq 间隙序列
     * @return 排序后的(值, 原始索引)列表
     */
    QVector<QPair<double, int>> sortWithIndex(
        const QVector<double>& data,
        bool ascending = true,
        GapSequence seq = GapSequence::Ciura) const;

    /**
     * @brief 生成指定类型的间隙序列
     * @param n 数据长度
     * @param seq 序列类型
     * @return 间隙值列表(降序)
     */
    QVector<int> generateGapSequence(int n, GapSequence seq) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 排序完成信号 */
    void sortCompleted(int elementCount, int comparisons, int swaps);

private:
    /** @brief Ciura序列 */
    QVector<int> ciuraSequence(int n) const;

    /** @brief Tokuda序列 */
    QVector<int> tokudaSequence(int n) const;

    /** @brief Hibbard序列 */
    QVector<int> hibbardSequence(int n) const;

    /** @brief Sedgewick序列 */
    QVector<int> sedgewickSequence(int n) const;

    /** @brief Shell原始序列 */
    QVector<int> shellSequence(int n) const;

    mutable Stats m_stats;           /**< 统计信息 */
    mutable double m_timeSum = 0.0;  /**< 累计时间 */
};
