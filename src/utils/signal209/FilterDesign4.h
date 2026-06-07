/**
 * @file FilterDesign4.h
 * @brief 数字滤波器设计(等波纹Parks-McClellan+WLS二阶约束) — Digital Filter Designer with Equiripple Parks-McClellan and WLS Second-Order Constraints
 *
 * 功能: 实现数字滤波器设计器，支持Parks-McClellan等波纹设计、
 *       加权最小二乘(WLS)二阶约束和频率响应分析。
 *
 * 协作: FIRFilter6(FIR滤波器) / IIRFilter5(IIR滤波器) / WindowFunction3(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 数字滤波器设计(等波纹Parks-McClellan+WLS二阶约束)
 */
class FilterDesign4 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter type */
    enum FilterType { LowPass, HighPass, BandPass, BandStop };

    /** @brief Band specification */
    struct BandSpec {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double gain = 1.0;
        double weight = 1.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalDesigns = 0;
        int filterOrder = 0;
        int numBands = 0;
        double designTimeMs = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FilterDesign4(QObject *parent = nullptr);
    ~FilterDesign4() override;

    void setFilterOrder(int order);
    void setFilterType(FilterType type);
    void setBands(const QVector<BandSpec>& bands);

    /** @brief Design equiripple filter using Remez exchange (Parks-McClellan) */
    QVector<double> designEquiripple();

    /** @brief Design using weighted least squares with second-order constraints */
    QVector<double> designWLS();

    /** @brief Compute frequency response at given frequencies */
    QVector<QPair<double, double>> frequencyResponse(
        const QVector<double>& coeffs, const QVector<double>& freqs) const;

    /** @brief Compute group delay */
    QVector<double> groupDelay(const QVector<double>& coeffs,
                                const QVector<double>& freqs) const;

    /** @brief Get designed coefficients */
    QVector<double> getCoefficients() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void designCompleted(int order, int numBands, double timeMs);

private:
    int m_order = 32;
    FilterType m_type = LowPass;
    QVector<BandSpec> m_bands;
    QVector<double> m_coeffs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Remez exchange iteration */
    QVector<double> remezExchange(int N, const QVector<double>& freqGrid,
                                   const QVector<double>& desired,
                                   const QVector<double>& weights) const;

    /** @brief Lagrange interpolation on extremal set */
    double lagrangeInterp(double freq, const QVector<double>& extFreqs,
                           const QVector<double>& extVals) const;

    /** @brief Dense frequency grid for Remez */
    void buildFreqGrid(int gridDensity, QVector<double>& freqs,
                        QVector<double>& desired, QVector<double>& weights) const;

    /** @brief Cosine basis evaluation */
    static double cosineBasis(int k, double freq);
};
