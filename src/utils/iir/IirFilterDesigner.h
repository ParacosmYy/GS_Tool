/**
 * @file IirFilterDesigner.h
 * @brief IIR滤波器设计器 — 巴特沃斯/切比雪夫
 *
 * 功能: 设计巴特沃斯/切比雪夫IIR滤波器，输出双二阶级联(biquad)系数，
 *       支持低通/高通/带通/带阻，统计设计次数/阶数/耗时。
 */
#ifndef IIRFILTERDESIGNER_H
#define IIRFILTERDESIGNER_H

#include <QObject>
#include <QVector>
#include <complex>

class IirFilterDesigner : public QObject {
    Q_OBJECT
public:
    enum class FilterType { LowPass, HighPass, BandPass, BandStop };
    enum class Approximation { Butterworth, ChebyshevType1, ChebyshevType2 };

    /** 双二阶节 */
    struct Biquad {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
        double a1 = 0.0, a2 = 0.0;
    };

    /** 设计统计 */
    struct Stats {
        quint64 totalDesigns = 0;
        quint64 totalSectionsGenerated = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit IirFilterDesigner(QObject* parent = nullptr);

    /** @brief 设计IIR滤波器 @param type 类型 @param approx 近似 @param order 阶数 @param cutoffNorm 归一化截止 @param rippleDb 纹波(dB) @return biquad级联 */
    QVector<Biquad> design(FilterType type, Approximation approx,
                           int order, double cutoffNorm,
                           double rippleDb = 1.0);

    /** @brief 频率响应 @param sections biquad级联 @param numPoints 点数 @return (频率,幅度dB) */
    QVector<QPair<double, double>> frequencyResponse(
        const QVector<Biquad>& sections, int numPoints = 512) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void designCompleted(int order, int sectionCount);

private:
    /** 巴特沃斯极点 */
    QVector<std::complex<double>> butterworthPoles(int order) const;
    /** 切比雪夫I型极点 */
    QVector<std::complex<double>> chebyshev1Poles(int order, double rippleDb) const;
    /** 极点→双二阶节 */
    QVector<Biquad> polesToBiquads(
        const QVector<std::complex<double>>& poles, FilterType type,
        double cutoffNorm) const;

    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // IIRFILTERDESIGNER_H
