/**
 * @file FilterDesign5.h
 * @brief IIR滤波器设计器(双线性变换+Butterworth/Chebyshev原型+频率预畸变) — IIR Filter Designer with Bilinear Transform and Butterworth/Chebyshev Prototype with Frequency Warping
 *
 * 功能: 实现IIR数字滤波器设计，通过双线性变换和频率预畸变，
 *       支持Butterworth和Chebyshev模拟原型转换为数字滤波器。
 *
 * 协作: FilterBank5(滤波器组) / Expander6(扩展器) / Goertzel7(频率检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief IIR滤波器设计器(双线性变换+Butterworth/Chebyshev原型)
 */
class FilterDesign5 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter type */
    enum FilterType { LowPass, HighPass, BandPass, BandStop };
    Q_ENUM(FilterType)

    /** @brief Prototype type */
    enum Prototype { Butterworth, ChebyshevType1 };
    Q_ENUM(Prototype)

    /** @brief Designed filter coefficients */
    struct FilterCoeffs {
        QVector<double> b;  // numerator (feedforward)
        QVector<double> a;  // denominator (feedback)
        int order = 0;
        FilterType type = LowPass;
        double cutoffFreq = 0.0;
    };

    /** @brief Frequency response at one point */
    struct FreqResponse {
        double frequency = 0.0;
        double magnitude = 0.0;  // dB
        double phase = 0.0;      // radians
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastOrder = 0;
        int sampleRate = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FilterDesign5(QObject *parent = nullptr);
    ~FilterDesign5() override;

    /** @brief Set sample rate */
    void setSampleRate(int sampleRate = 44100);

    /** @brief Design filter with given specifications */
    FilterCoeffs design(FilterType type, Prototype proto, int order,
                        double cutoffFreq, double rippleDb = 0.5) const;

    /** @brief Design bandpass/bandstop filter */
    FilterCoeffs designBand(FilterType type, Prototype proto, int order,
                            double lowFreq, double highFreq,
                            double rippleDb = 0.5) const;

    /** @brief Apply filter to signal */
    QVector<double> apply(const QVector<double>& input,
                          const FilterCoeffs& coeffs) const;

    /** @brief Compute frequency response */
    QVector<FreqResponse> frequencyResponse(const FilterCoeffs& coeffs,
                                             int numPoints = 512) const;

    /** @brief Pre-warp frequency for bilinear transform */
    double prewarpFrequency(double freq) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterDesigned(int order, double cutoff, double timeMs);

private:
    int m_sampleRate = 44100;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Butterworth analog prototype poles for order n */
    QVector<QVector<double>> butterworthPoles(int n) const;

    /** @brief Chebyshev Type I analog prototype poles */
    QVector<QVector<double>> chebyshevPoles(int n, double ripple) const;

    /** @brief Convert analog poles to digital via bilinear transform */
    void bilinearTransform(QVector<double>& b, QVector<double>& a,
                           const QVector<QVector<double>>& poles,
                           double warpedFreq) const;

    /** @brief Polynomial multiplication (convolution) */
    QVector<double> polyConvolve(const QVector<double>& p1,
                                  const QVector<double>& p2) const;

    /** @brief Expand (s - pole) factors into polynomial coefficients */
    QVector<double> expandPoles(const QVector<QVector<double>>& poles) const;
};
