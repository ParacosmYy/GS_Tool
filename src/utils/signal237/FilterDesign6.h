/**
 * @file FilterDesign6.h
 * @brief FIR滤波器设计(Parks-McClellan等波纹+加权切比雪夫逼近最优系数) — FIR Filter Design with Parks-McClellan Equiripple and Weighted Chebyshev Approximation for Optimal Filter Coefficients
 *
 * 功能: 实现FIR数字滤波器设计(FIR filter design)，采用Parks-McClellan等波纹算法
 *       (Parks-McClellan equiripple algorithm)与加权切比雪夫逼近(weighted Chebyshev
 *       approximation)，计算最优滤波器系数(optimal filter coefficients)。
 *
 * 协作: Goertzel8(Goertzel算法) / WindowFunction4(窗函数) / IirFilter5(IIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief FIR滤波器设计(Parks-McClellan等波纹+加权切比雪夫逼近)
 */
class FilterDesign6 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter type */
    enum FilterType { LowPass, HighPass, BandPass, BandStop };

    /** @brief Band specification */
    struct BandSpec {
        double lowFreq = 0.0;
        double highFreq = 0.0;
        double weight = 1.0;
        bool isPassband = true;
    };

    /** @brief Design result */
    struct DesignResult {
        QVector<double> coefficients;
        int filterOrder = 0;
        double actualRippleDb = 0.0;
        double actualStopbandDb = 0.0;
        int iterationsUsed = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastFilterOrder = 0;
        int numDesigns = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FilterDesign6(QObject *parent = nullptr);
    ~FilterDesign6() override;

    /** @brief Set filter order (number of taps - 1) */
    void setFilterOrder(int order);

    /** @brief Set passband ripple in dB */
    void setPassbandRipple(double db);

    /** @brief Set stopband attenuation in dB */
    void setStopbandAttenuation(double db);

    /** @brief Set max Remez iterations */
    void setMaxIterations(int iter);

    /** @brief Design filter from band specifications */
    DesignResult design(const QVector<BandSpec>& bands, FilterType type);

    /** @brief Compute frequency response at given frequencies */
    QVector<double> frequencyResponse(const QVector<double>& coeffs,
                                       const QVector<double>& freqs) const;

    /** @brief Design windowed FIR as fallback */
    QVector<double> designWindowedFIR(int taps, double cutoff, FilterType type) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void remezIteration(int iter, double error);
    void designCompleted(int order, double ripple, double timeMs);

private:
    int m_filterOrder = 50;
    double m_passbandRipple = 0.1;
    double m_stopbandAttenuation = 60.0;
    int m_maxIterations = 50;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Remez exchange algorithm core */
    QVector<double> remezExchange(const QVector<double>& freqGrid,
                                  const QVector<double>& desiredGrid,
                                  const QVector<double>& weightGrid,
                                  int numTaps);

    /** @brief Compute Lagrange interpolation weight */
    double lagrangeWeight(int k, const QVector<double>& extFreqs, int numExtrema) const;

    /** @brief Evaluate frequency response of candidate filter */
    double evaluateResponse(double freq, const QVector<double>& coeffs) const;

    /** @brief Sinc function */
    static double sinc(double x);
};
