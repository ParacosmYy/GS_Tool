/**
 * @file MannWhitneyTest.h
 * @brief Mann-Whitney U检验 — 两组独立样本的非参数检验
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class MannWhitneyTest
 * @brief 实现Mann-Whitney U检验（也称Wilcoxon秩和检验）
 *
 * 用于检验两组独立样本是否来自同一分布。
 * 不假设正态分布，是非参数检验。
 */
class MannWhitneyTest : public QObject {
    Q_OBJECT
public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalTests = 0;           ///< 总检验次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit MannWhitneyTest(QObject* parent = nullptr);

    /**
     * @brief 执行Mann-Whitney U检验
     * @param group1 第一组样本数据
     * @param group2 第二组样本数据
     * @return U统计量（较小值）
     */
    double test(const QVector<double>& group1, const QVector<double>& group2);

    /** @brief 获取最近一次检验的p值 */
    double pValue() const { return m_pValue; }

    /** @brief 获取最近一次检验的U统计量 */
    double uStatistic() const { return m_uStatistic; }

    /** @brief 获取最近一次检验的效应量(r = Z/sqrt(N)) */
    double effectSize() const { return m_effectSize; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 检验完成信号 @param u U统计量 @param p p值 */
    void testCompleted(double u, double p);

private:
    /**
     * @brief 计算正态分布CDF（近似）
     * @param z 标准正态Z值
     * @return 累积概率
     */
    double normalCdf(double z) const;

    /**
     * @brief 对合并样本进行排名，处理并列值取平均秩
     * @param combined 合并后的数据
     * @param n1 第一组大小
     * @return 第一组排名和、第二组排名和
     */
    QPair<double, double> computeRanks(const QVector<double>& combined,
                                       int n1) const;

    Stats  m_stats;
    double m_timeSum     = 0.0;   ///< 累计处理时间
    double m_pValue      = 1.0;   ///< 最近p值
    double m_uStatistic  = 0.0;   ///< 最近U统计量
    double m_effectSize  = 0.0;   ///< 最近效应量
};
