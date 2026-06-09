/**
 * @file FilterDesign7.h
 * @brief IIR滤波器设计(双线性变换模拟原型+巴特沃斯/切比雪夫/椭圆响应匹配) — IIR Filter Design with Bilinear Transform from Analog Prototypes and Butterworth/Chebyshev/Elliptic Response Matching
 *
 * 功能: 实现IIR滤波器设计(IIR Filter Design)，使用双线性变换(bilinear
 *       transform)将模拟原型转换为数字滤波器，支持巴特沃斯(Butterworth)、
 *       切比雪夫I型(Chebyshev Type I)和椭圆(Elliptic)响应匹配。
 *
 * 协作: Equalizer10(均衡器) / Expander10(扩展器) / Compressor10(压缩器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief IIR滤波器设计(双线性变换+多种响应匹配)
 */
class FilterDesign7 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter type enumeration */
    enum FilterType { LowPass, HighPass, BandPass, BandStop };
    Q_ENUM(FilterType)

    /** @brief Response prototype enumeration */
    enum ResponseType { Butterworth, ChebyshevI, Elliptic };
    Q_ENUM(ResponseType)

    /** @brief Filter coefficients */
    struct Coefficients {
        QVector<double> b;  // Numerator coefficients
        QVector<double> a;  // Denominator coefficients
        int order = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numDesigns = 0;
        int filterOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FilterDesign7(QObject *parent = nullptr);
    ~FilterDesign7() override;

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Design filter with given specifications */
    Coefficients design(FilterType type, ResponseType response,
                        int order, double cutoffFreq,
                        double rippleDb = 0.5, double stopbandDb = 40.0);

    /** @brief Design bandpass/bandstop with two cutoffs */
    Coefficients designBand(FilterType type, ResponseType response,
                             int order, double lowFreq, double highFreq,
                             double rippleDb = 0.5, double stopbandDb = 40.0);

    /** @brief Apply filter to a block of samples */
    QVector<double> apply(const Coefficients& coeff,
                           const QVector<double>& input);

    /** @brief Compute frequency response at given frequencies */
    QVector<QVector<double>> frequencyResponse(const Coefficients& coeff,
                                                 const QVector<double>& freqs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void designCompleted(int order, int type, double timeMs);

private:
    double m_sampleRate = 44100.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Pre-warp frequency for bilinear transform */
    double prewarp(double freq) const;

    /** @brief Butterworth analog prototype poles */
    QVector<QVector<double>> butterworthPoles(int order) const;

    /** @brief Chebyshev Type I analog prototype poles */
    QVector<QVector<double>> chebyshevPoles(int order, double ripple) const;

    /** @brief Elliptic analog prototype poles (approximation) */
    QVector<QVector<double>> ellipticPoles(int order, double ripple,
                                            double stopband) const;

    /** @brief Bilinear transform: analog pole -> digital pole */
    QPair<QVector<double>, QVector<double>> bilinearTransform(
        const QVector<QVector<double>>& analogPoles,
        double warpedCutoff) const;

    /** @brief Build transfer function from pole-zero pairs */
    Coefficients buildCoefficients(
        const QVector<double>& zerosReal, const QVector<double>& zerosImag,
        const QVector<double>& polesReal, const QVector<double>& polesImag) const;

    /** @brief Evaluate polynomial at z = exp(j*w) */
    QPair<double, double> evalPoly(const QVector<double>& coeffs,
                                    double freq) const;
};
