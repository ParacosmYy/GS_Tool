/**
 * @file FilterDesign10.h
 * @brief 滤波器设计(最小二乘FIR逼近与Parks-McClellan等波纹交换实现最优线性相位滤波器综合) — Filter Design with Least-squares FIR Approximation and Parks-McClellan Equiripple Exchange for Optimal Linear-phase Filter Synthesis
 *
 * 功能: 实现滤波器设计(filter design)，采用最小二乘FIR逼近(least-squares FIR approximation)
 *       与Parks-McClellan等波纹交换(Parks-McClellan equiripple exchange)实现最优线性相位滤波器综合(optimal linear-phase filter synthesis)。
 *
 * 协作: FilterIIR10(IIR滤波器) / WindowFunction10(窗函数) / PolyphaseFilterbank10(多相滤波器组)
 */
#pragma once

#include <QObject>
#include <QVector>

class FilterDesign10 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter type */
    enum FilterType { LowPass = 0, HighPass = 1, BandPass = 2, BandStop = 3 };

    /** @brief Design specification */
    struct Spec {
        FilterType type = LowPass;
        int order = 50;                  // Filter order (taps - 1)
        double sampleRate = 44100.0;
        double freq1 = 1000.0;           // Cutoff / low freq (Hz)
        double freq2 = 3000.0;           // High freq for band-pass/stop
        double rippleDB = 1.0;           // Passband ripple (dB)
        double stopDB = 40.0;            // Stopband attenuation (dB)
    };

    /** @brief Design result */
    struct DesignResult {
        QVector<double> coefficients;    // FIR filter taps
        QVector<double> freqResponse;    // Frequency response magnitude
        double actualRipple = 0.0;
        double actualStopband = 0.0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalDesigns = 0;
        int lastOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FilterDesign10(QObject *parent = nullptr);
    ~FilterDesign10() override;

    /** @brief Design FIR filter using least-squares method */
    DesignResult designLeastSquares(const Spec& spec);

    /** @brief Design FIR filter using Parks-McClellan (Remez exchange) */
    DesignResult designParksMcClellan(const Spec& spec);

    /** @brief Compute frequency response of filter coefficients */
    QVector<double> frequencyResponse(const QVector<double>& coeffs,
                                        int numPoints, double sampleRate) const;

    /** @brief Apply filter to signal (convolution) */
    QVector<double> applyFilter(const QVector<double>& signal,
                                  const QVector<double>& coeffs) const;

    /** @brief Estimate minimum filter order for given specs */
    int estimateOrder(const Spec& spec) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void designDone(int order, double ripple, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Normalized frequency (0 to 0.5 = Nyquist) */
    double normalizeFreq(double freqHz, double sr) const;

    /** @brief Compute ideal frequency response at given normalized freq */
    double idealResponse(double f, const Spec& spec) const;

    /** @brief Weight function for Parks-McClellan */
    double weightFunction(double f, const Spec& spec) const;

    /** @brief Remez exchange iteration */
    bool remezExchange(QVector<double>& h, int order,
                        const QVector<double>& freqGrid,
                        const QVector<double>& desired,
                        const QVector<double>& weights,
                        int maxIter);

    /** @brief Lagrange interpolation on the extremal set */
    double lagrangeInterp(double x, const QVector<double>& xData,
                            const QVector<double>& yData) const;
};
