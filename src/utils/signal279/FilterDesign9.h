/**
 * @file FilterDesign9.h
 * @brief 滤波器设计(最小二乘最优FIR与频率采样法的任意幅度响应规范) — Filter Design with Least-squares Optimal FIR and Frequency Sampling Method for Arbitrary Magnitude Response Specifications
 *
 * 功能: 实现滤波器设计(Filter design)，采用最小二乘最优FIR(least-squares optimal FIR)
 *       与频率采样法(frequency sampling method)实现任意幅度响应规范(arbitrary magnitude response specifications)。
 *
 * 协作: WindowFunction7(窗函数) / Goertzel11(Goertzel算法) / IIRDesigner6(IIR设计)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 滤波器设计(最小二乘最优FIR与频率采样法)
 */
class FilterDesign9 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter type */
    enum FilterType { LowPass, HighPass, BandPass, BandStop, Arbitrary };

    /** @brief Frequency-magnitude specification point */
    struct SpecPoint {
        double freq = 0.0;    // Normalized frequency [0, 0.5]
        double magnitude = 0.0;
        double weight = 1.0;
    };

    /** @brief Design result */
    struct DesignResult {
        QVector<double> coefficients;  // FIR filter taps
        int order = 0;                  // Filter order
        FilterType type = LowPass;
        QVector<double> freqResponse;   // Magnitude response
        QVector<double> phaseResponse;  // Phase response
        double ripplePassband = 0.0;
        double rippleStopband = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numDesigns = 0;
        int maxOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FilterDesign9(QObject *parent = nullptr);
    ~FilterDesign9() override;

    /** @brief Design least-squares optimal FIR filter */
    DesignResult designLeastSquares(int order, FilterType type,
                                      double cutoff1, double cutoff2 = 0.0);

    /** @brief Design FIR filter using frequency sampling method */
    DesignResult designFrequencySampling(int order,
                                           const QVector<SpecPoint>& spec);

    /** @brief Design arbitrary response FIR via weighted least-squares */
    DesignResult designArbitrary(int order,
                                   const QVector<SpecPoint>& spec);

    /** @brief Compute frequency response of filter coefficients */
    void frequencyResponse(const QVector<double>& coeffs,
                             int numPoints,
                             QVector<double>& magnitude,
                             QVector<double>& phase) const;

    /** @brief Apply window to coefficients */
    QVector<double> applyWindow(const QVector<double>& coeffs, const QString& windowType) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void designComplete(int order, FilterType type, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build ideal frequency response for standard filter types */
    void idealResponse(int n, FilterType type, double cutoff1, double cutoff2,
                        QVector<double>& H) const;

    /** @brief Build weight function for transition bands */
    QVector<double> buildWeights(int n, FilterType type,
                                   double cutoff1, double cutoff2) const;

    /** @brief Sinc function */
    static double sinc(double x);

    /** @brief Generate window coefficients */
    QVector<double> generateWindow(int n, const QString& type) const;
};
