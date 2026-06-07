/**
 * @file FilterDesign3.h
 * @brief IIR滤波器设计(椭圆Cauer逼近+双线性变换+零极点分析) — IIR Filter Designer with Elliptic (Cauer) Approximation via Bilinear Transform and Pole-Zero Analysis
 *
 * 功能: 实现IIR滤波器设计，支持椭圆(Cauer)逼近、
 *       双线性变换预畸变、零极点分析和频率响应计算。
 *
 * 协作: FilterDesign2(Butterworth) / Convolver2(卷积) / WindowFunction4(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QtMath>

/**
 * @brief IIR滤波器设计(椭圆Cauer逼近+双线性变换+零极点分析)
 */
class FilterDesign3 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter type */
    enum class FilterType { LowPass, HighPass, BandPass, BandStop };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalDesigns = 0;
        int filterOrder = 0;
        double passbandRipple = 0.0;
        double stopbandAtten = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Pole-zero analysis result */
    struct PoleZero {
        QVector<double> polesRe;
        QVector<double> polesIm;
        QVector<double> zerosRe;
        QVector<double> zerosIm;
    };

    explicit FilterDesign3(QObject *parent = nullptr);
    ~FilterDesign3() override;

    void setFilterType(FilterType type);
    void setOrder(int n);
    void setPassbandRipple(double rippleDb);
    void setStopbandAttenuation(double attenDb);

    /** @brief Design elliptic filter, return (b, a) coefficients */
    QPair<QVector<double>, QVector<double>> design(double Wn) const;

    /** @brief Complete elliptic integral of the first kind K(k) */
    static double ellipticK(double k);

    /** @brief Jacobian elliptic function sn(u, k) */
    static double ellipticSn(double u, double k);

    /** @brief Bilinear transform: analog to digital */
    QPair<QVector<double>, QVector<double>> bilinearTransform(
        const QVector<double>& bAna, const QVector<double>& aAna,
        double fs) const;

    /** @brief Frequency response H(e^jw) */
    void frequencyResponse(const QVector<double>& b, const QVector<double>& a,
                            const QVector<double>& w,
                            QVector<double>& mag, QVector<double>& phase) const;

    /** @brief Compute pole-zero locations */
    PoleZero poleZeroAnalysis(const QVector<double>& b,
                               const QVector<double>& a) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void designCompleted(int order, double ripple, double atten, double timeMs);

private:
    FilterType m_type = FilterType::LowPass;
    int m_order = 4;
    double m_passRipple = 1.0;   // dB
    double m_stopAtten = 40.0;   // dB

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute elliptic analog prototype poles */
    QVector<QPair<double, double>> analogEllipticPoles() const;

    /** @brief Solve quadratic for polynomial roots (Bairstow) */
    QVector<QPair<double, double>> polyRoots(const QVector<double>& coeffs) const;

    /** @brief Evaluate polynomial at complex point */
    static QPair<double, double> polyEval(const QVector<double>& c,
                                           double re, double im);
};
