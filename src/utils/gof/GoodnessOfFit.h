/**
 * @file GoodnessOfFit.h
 * @brief 拟合优度检验 — 卡方/KS/AD/正态性检验
 *
 * 功能: 提供卡方拟合检验、KS检验、Anderson-Darling正态性检验，
 *       统计检验次数/拒绝/接受数/耗时。
 */
#ifndef GOODNESSOFFIT_H
#define GOODNESSOFFIT_H

#include <QObject>
#include <QVector>

class GoodnessOfFit : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalTests = 0;
        quint64 totalRejections = 0;
        quint64 totalAcceptances = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    /** 检验结果 */
    struct TestResult {
        double statistic = 0.0;    ///< 检验统计量
        double pValue = 0.0;       ///< p值
        bool rejected = false;     ///< 是否拒绝原假设
        double significance = 0.05;///< 显著性水平
        QString testName;          ///< 检验名称
    };

    explicit GoodnessOfFit(QObject* parent = nullptr);

    /** @brief 卡方拟合检验 @param observed 观测频数 @param expected 期望频数 @param significance 显著性水平 @return 检验结果 */
    TestResult chiSquareTest(const QVector<double>& observed,
                              const QVector<double>& expected,
                              double significance = 0.05);

    /** @brief 单样本KS检验(正态) @param data 数据 @param significance 显著性水平 @return 检验结果 */
    TestResult ksNormalityTest(const QVector<double>& data,
                               double significance = 0.05);

    /** @brief Jarque-Bera正态性检验 @param data 数据 @param significance 显著性水平 @return 检验结果 */
    TestResult jarqueBeraTest(const QVector<double>& data,
                              double significance = 0.05);

    /** @brief 计算正态CDF近似 @param z 标准正态值 @return 概率 */
    double normalCDF(double z) const;

    /** @brief 计算卡方分布CDF近似 @param x 统计量 @param df 自由度 @return 概率 */
    double chiSquareCDF(double x, int df) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void testCompleted(const QString& testName, double statistic, double pValue);

private:
    /** 不完全Gamma函数(下) */
    double lowerIncompleteGamma(double s, double x) const;
    /** Gamma函数(Stirling近似) */
    double gammaFunc(double x) const;

    Stats m_stats;
    double m_timeSum;
};

#endif // GOODNESSOFFIT_H
