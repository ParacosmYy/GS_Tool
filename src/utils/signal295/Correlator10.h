/**
 * @file Correlator10.h
 * @brief 相关器(重叠保留块处理与归一化互相关及统计显著性检验实现信号匹配) — Correlator with Overlap-save Block Processing and Normalized Cross-correlation with Statistical Significance Testing for Signal Matching
 *
 * 功能: 实现相关器(correlator)，采用重叠保留块处理(overlap-save block processing)
 *       与归一化互相关(normalized cross-correlation)及统计显著性检验(statistical significance testing)实现信号匹配(signal matching)。
 *
 * 协作: MatchedFilter10(匹配滤波器) / Convolver11(卷积器) / PeakDetector9(峰值检测)
 */
#pragma once

#include <QObject>
#include <QVector>

class Correlator10 : public QObject {
    Q_OBJECT

public:
    /** @brief Correlation result */
    struct CorrelResult {
        QVector<double> correlation;      // Normalized cross-correlation
        double peakValue = 0.0;           // Peak correlation value
        int peakLag = 0;                  // Lag at peak
        double significance = 0.0;        // P-value of peak
        double zScore = 0.0;              // Z-score of peak
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalCorrelations = 0;
        int blockSize = 0;
        double avgPeakCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Correlator10(QObject *parent = nullptr);
    ~Correlator10() override;

    void setBlockSize(int size);
    void setSignificanceLevel(double alpha);     // Significance level for testing

    /** @brief Compute normalized cross-correlation via overlap-save */
    CorrelResult correlate(const QVector<double>& signal,
                            const QVector<double>& pattern) const;

    /** @brief Compute cross-correlation at a specific lag range */
    QVector<double> correlateLagRange(const QVector<double>& signal,
                                        const QVector<double>& pattern,
                                        int minLag, int maxLag) const;

    /** @brief Statistical significance test for correlation peak */
    double computePValue(double r, int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationDone(int sigLen, int patLen, double peak, double timeMs);

private:
    int m_blockSize = 1024;
    double m_alpha = 0.05;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_peakSum = 0.0;

    /** @brief Compute mean of a vector */
    double mean(const QVector<double>& v) const;

    /** @brief Compute standard deviation */
    double stddev(const QVector<double>& v) const;

    /** @brief Overlap-save block convolution */
    QVector<double> overlapSaveConvolve(const QVector<double>& signal,
                                          const QVector<double>& kernel) const;

    /** @brief Inverse of Student's t CDF approximation (two-tailed p-value) */
    double tDistPValue(double t, int df) const;

    /** @brief Gamma function approximation (Stirling) */
    double gammaLn(double x) const;

    /** @brief Regularized incomplete beta function */
    double betaIncomplete(double a, double b, double x) const;
};
