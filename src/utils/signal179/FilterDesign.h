/**
 * @file FilterDesign.h
 * @brief IIR/FIR滤波器设计(Butterworth/Chebyshev I&II/Elliptic+双线性变换+频率响应) — IIR/FIR Filter Designer: Butterworth/Chebyshev I&II/Elliptic with Bilinear Transform and Freq Response
 *
 * 功能: 实现IIR/FIR滤波器设计器，支持Butterworth/Chebyshev I&II/
 *       Elliptic滤波器、双线性变换频率预畸变和频率响应计算。
 *
 * 协作: FftEngine(FFT) / WindowFunction4(窗函数) / Expander2(扩展器)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief IIR/FIR滤波器设计器
 */
class FilterDesign : public QObject {
    Q_OBJECT

public:
    /** @brief 滤波器类型 */
    enum Type { LowPass = 0, HighPass = 1, BandPass = 2, BandStop = 3 };

    /** @brief 近似方法 */
    enum Approximation { Butterworth = 0, ChebyshevI = 1,
                         ChebyshevII = 2, Elliptic = 3 };

    /** @brief 滤波器系数 */
    struct Coefficients {
        QVector<double> b;  ///< 分子系数(FIR: all; IIR: feedforward)
        QVector<double> a;  ///< 分母系数(IIR: feedback; FIR: {1})
        int order = 0;
        Type type = LowPass;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDesigns = 0;
        int filterOrder = 0;
        int numCoeffs = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FilterDesign(QObject *parent = nullptr);
    ~FilterDesign() override;

    void setOrder(int order);
    void setSampleRate(double fs);
    void setCutoffFreq(double fc);
    void setBandwidth(double lowFc, double highFc);
    void setPassbandRipple(double dB);
    void setStopbandAttenuation(double dB);

    /** @brief 设计滤波器 */
    Coefficients design(Type type, Approximation approx);

    /** @brief 双线性变换预畸变频率 */
    double prewarp(double analogFreq) const;

    /** @brief 计算频率响应 */
    QVector<QPair<double, double>> frequencyResponse(const Coefficients& coeff,
                                                      int numPoints = 512) const;

    /** @brief 应用FIR滤波器 */
    QVector<double> applyFIR(const QVector<double>& input,
                              const Coefficients& coeff) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void designCompleted(int order, int numB, int numA);

private:
    int m_order = 4;
    double m_fs = 44100.0;
    double m_fc = 1000.0;
    double m_fcLow = 500.0;
    double m_fcHigh = 2000.0;
    double m_passbandRipple = 1.0;    ///< dB
    double m_stopbandAtten = 40.0;    ///< dB

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Butterworth极点位置 */
    QVector<QPair<double, double>> butterworthPoles(int n) const;

    /** @brief 将s平面极点转换到z平面(双线性变换) */
    QVector<QPair<double, double>> bilinearTransform(
        const QVector<QPair<double, double>>& poles) const;
};
