/**
 * @file FilterDesign2.h
 * @brief 滤波器设计(Parks-McClellan等波纹FIR+最小阶估计) — Filter Designer with Parks-McClellan Equiripple FIR and Minimum-Order Estimation
 *
 * 功能: 实现FIR滤波器设计，支持Parks-McClellan等波纹算法、
 *       最小阶估计和频率响应分析。
 *
 * 协作: BiquadFilter6(双二阶) / WindowFunc5(窗函数) / FftEngine3(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 滤波器设计器(Parks-McClellan+最小阶估计)
 */
class FilterDesign2 : public QObject {
    Q_OBJECT

public:
    enum FilterType { LowPass, HighPass, BandPass, BandStop };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDesigns = 0;
        int filterOrder = 0;
        double rippleDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FilterDesign2(QObject *parent = nullptr);
    ~FilterDesign2() override;

    void setFilterType(FilterType type);
    void setSampleRate(double sr);
    void setRippleDb(double ripple);

    /** @brief Design equiripple FIR filter with given order */
    QVector<double> design(int order,
                            const QVector<QPair<double, double>>& bands,
                            const QVector<double>& desired,
                            const QVector<double>& weights);

    /** @brief Estimate minimum filter order for specs */
    int estimateOrder(double passbandFreq, double stopbandFreq,
                       double passbandRipple, double stopbandAtten);

    /** @brief Compute frequency response of designed filter */
    QVector<QVector<double>> frequencyResponse(const QVector<double>& coeffs,
                                                 int numPoints) const;

    /** @brief Get last designed coefficients */
    QVector<double> coefficients() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void designCompleted(int order, double ripple, double timeMs);

private:
    FilterType m_type = LowPass;
    double m_sampleRate = 44100.0;
    double m_rippleDb = 0.1;

    QVector<double> m_coeffs;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Parks-McClellan Remez exchange algorithm */
    QVector<double> remez(int order, int numBands,
                           const QVector<double>& bandEdges,
                           const QVector<double>& desired,
                           const QVector<double>& weights);

    /** @brief Compute delta (error) for reference set */
    double computeDelta(int r, const QVector<double>& xRef,
                         const QVector<double>& dRef,
                         const QVector<double>& wRef,
                         QVector<double>& alpha) const;

    /** @brief Evaluate Lagrange interpolant at point x */
    double evalInterpolant(double x, int r,
                            const QVector<double>& xRef,
                            const QVector<double>& alpha) const;
};
