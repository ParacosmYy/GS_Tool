/**
 * @file FriedmanTest.h
 * @brief Friedman检验 — 重复测量非参数检验
 *
 * 功能: 对多组相关样本进行Friedman秩和检验，
 *       计算卡方统计量和p值，统计检验次数/耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

class FriedmanTest : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalTests = 0;       ///< 总检验次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit FriedmanTest(QObject* parent = nullptr);

    /**
     * @brief 执行Friedman检验
     * @param groups 多组观测数据，每组一行(subject)，每列一个处理
     * @return 卡方统计量
     */
    double test(const QVector<QVector<double>>& groups);

    /** @brief 获取最近一次检验的p值 */
    double pValue() const;

    /** @brief 获取最近一次检验的卡方统计量 */
    double chiSquared() const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 检验完成信号 @param chi2 卡方值 @param p p值 */
    void testCompleted(double chi2, double p);

private:
    /** @brief 计算每行的秩(排名) */
    QVector<double> computeRanks(const QVector<double>& row) const;

    /** @brief 利用Chi-square CDF近似计算p值(不完全Gamma函数) */
    double chiSquareCDF(double x, double df) const;

    /** @brief Lanczos近似计算Gamma函数 */
    double gammaFunction(double x) const;

    /** @brief 不完全Gamma函数(级数展开) */
    double incompleteGamma(double s, double x) const;

    /** @brief 计算同值校正因子(tie correction) */
    double computeTieCorrection(const QVector<double>& row) const;

    Stats  m_stats;
    double m_timeSum;
    double m_lastChi2;   ///< 最近一次卡方值
    double m_lastPValue; ///< 最近一次p值
};
