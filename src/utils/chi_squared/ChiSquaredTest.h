/**
 * @file ChiSquaredTest.h
 * @brief 卡方检验(Chi-Squared Test)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class ChiSquaredTest
 * @brief 卡方检验 — 拟合优度和独立性检验
 *
 * 支持拟合优度检验(单变量)、独立性检验(列联表)、
 * 卡方分布CDF计算。
 * 适用于统计假设检验、数据分布验证等场景。
 */
class ChiSquaredTest : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalTests = 0;       /**< 总检验次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit ChiSquaredTest(QObject* parent = nullptr);

    /**
     * @brief 拟合优度检验(Goodness of Fit)
     * @param observed 观测频数
     * @param expected 期望频数(空则均匀分布)
     * @return 卡方统计量
     */
    double goodnessOfFit(const QVector<double>& observed,
                          const QVector<double>& expected = {});

    /**
     * @brief 独立性检验(列联表)
     * @param contingencyTable 行×列频数表
     * @return 卡方统计量
     */
    double independence(const QVector<QVector<double>>& contingencyTable);

    /**
     * @brief 计算p值
     * @param chi2 卡方统计量
     * @param df 自由度
     * @return p值
     */
    static double pValue(double chi2, int df);

    /**
     * @brief 卡方分布CDF
     * @param x 卡方值
     * @param df 自由度
     * @return 累积概率
     */
    static double cdf(double x, int df);

    /**
     * @brief Yate连续性校正卡方
     * @param a,b,c,d 2×2列联表四格
     * @return 校正后卡方值
     */
    static double yatesCorrection(double a, double b, double c, double d);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 检验完成信号 */
    void testCompleted(double chi2, int df, double p);

private:
    static double gammaLn(double x);
    static double lowerIncompleteGamma(double x, double a);

    Stats m_stats;
    double m_timeSum;
};
