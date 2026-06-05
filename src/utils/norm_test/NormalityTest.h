/**
 * @file NormalityTest.h
 * @brief 正态性检验工具
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 正态性检验类,支持多种检验方法
 *
 * 提供Shapiro-Wilk、Anderson-Darling、Jarque-Bera等
 * 正态分布假设检验方法,用于数据质量评估。
 */
class NormalityTest : public QObject
{
    Q_OBJECT

public:
    /** @brief 检验结果 */
    struct TestResult {
        QString testName;       ///< 检验名称
        double statistic = 0.0; ///< 检验统计量
        double pValue = 0.0;    ///< p值
        bool isNormal = false;  ///< 是否通过正态性(α=0.05)
        int sampleSize = 0;     ///< 样本大小
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalTests = 0;             ///< 总检验次数
        int totalNormalDetected = 0;    ///< 检测为正态的次数
        int totalNonNormalDetected = 0; ///< 检测为非正态的次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit NormalityTest(QObject* parent = nullptr);

    /**
     * @brief Shapiro-Wilk W检验
     * @param data 样本数据(至少3个)
     * @return 检验结果
     */
    TestResult shapiroWilk(const QVector<double>& data);

    /**
     * @brief Jarque-Bera检验
     * @param data 样本数据
     * @return 检验结果
     */
    TestResult jarqueBera(const QVector<double>& data);

    /**
     * @brief Anderson-Darling检验
     * @param data 样本数据
     * @return 检验结果
     */
    TestResult andersonDarling(const QVector<double>& data);

    /**
     * @brief D'Agostino K²综合检验
     * @param data 样本数据(至少20个)
     * @return 检验结果
     */
    TestResult dagostinoK2(const QVector<double>& data);

    /**
     * @brief 综合检验(全部方法)
     * @param data 样本数据
     * @return 多种检验结果列表
     */
    QVector<TestResult> comprehensiveTest(const QVector<double>& data);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 检验完成信号 */
    void testCompleted(const QString& testName, bool isNormal);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    double computeMean(const QVector<double>& data) const;
    double computeVariance(const QVector<double>& data, double mean) const;
    double computeSkewness(const QVector<double>& data, double mean,
                           double stdDev) const;
    double computeKurtosis(const QVector<double>& data, double mean,
                           double stdDev) const;
    double normalCDF(double x) const;
    double chi2CDF(double x, int df) const;
    QVector<double> sortData(const QVector<double>& data) const;
};
