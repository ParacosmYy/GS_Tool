/**
 * @file KruskalWallisTest.h
 * @brief Kruskal-Wallis H检验 — 多组独立样本的非参数检验
 */
#pragma once

#include <QObject>
#include <QVector>
#include <vector>

/**
 * @class KruskalWallisTest
 * @brief Kruskal-Wallis H检验（单因素非参数方差分析）
 *
 * 检验多组独立样本是否来自同一分布。
 * 是Mann-Whitney U检验的多组推广。
 * 当组数=2时退化为Mann-Whitney检验。
 */
class KruskalWallisTest : public QObject {
    Q_OBJECT
public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalTests = 0;            ///< 总检验次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit KruskalWallisTest(QObject* parent = nullptr);

    /**
     * @brief 执行Kruskal-Wallis H检验
     * @param groups 各组数据（至少2组，每组至少1个数据）
     * @return H统计量
     */
    double test(const QVector<QVector<double>>& groups);

    /** @brief 获取最近一次检验的p值 */
    double pValue() const { return m_pValue; }

    /** @brief 获取最近一次检验的H统计量 */
    double hStatistic() const { return m_hStatistic; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 检验完成信号 @param h H统计量 @param p p值 */
    void testCompleted(double h, double p);

private:
    /** @brief 带组标记的值 */
    struct ValueGroup {
        double value;
        int    group;
    };

    Stats  m_stats;
    double m_timeSum     = 0.0;  ///< 累计处理时间
    double m_hStatistic  = 0.0;  ///< 最近H统计量
    double m_pValue      = 1.0;  ///< 最近p值

    /** @brief 合并并排序所有组数据 */
    std::vector<ValueGroup> mergeAndSort(
        const QVector<QVector<double>>& groups) const;

    /** @brief 分配排名（并列取平均） */
    std::vector<double> assignRanks(
        const std::vector<ValueGroup>& sorted) const;

    /** @brief 计算H统计量 */
    double computeH(const std::vector<double>& ranks,
                    const std::vector<ValueGroup>& sorted,
                    int k, int totalN) const;

    /** @brief 卡方分布上尾概率 */
    double chiSqPValue(double x, double df) const;

    /** @brief 不完全Gamma函数（下）近似 */
    double incompleteGamma(double a, double x) const;

    /** @brief Gamma函数对数（Lanczos近似） */
    double gammaLn(double x) const;
};
