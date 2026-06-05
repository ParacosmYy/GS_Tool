/**
 * @file ChebyshevFilter.h
 * @brief 切比雪夫I型滤波器设计 — 等纹波通带
 *
 * 功能: 切比雪夫I型IIR滤波器设计，支持可控通带纹波，
 *       低通/高通/带通/带阻，统计设计次数/应用次数/耗时。
 */
#ifndef CHEBYSHEVFILTER_H
#define CHEBYSHEVFILTER_H

#include <QObject>
#include <QVector>

class ChebyshevFilter : public QObject {
    Q_OBJECT
public:
    enum Type { LowPass, HighPass, BandPass, BandStop };

    struct Stats {
        quint64 totalDesigns = 0;
        quint64 totalApplications = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit ChebyshevFilter(QObject* parent = nullptr);

    /** @brief 设计滤波器 @param type 类型 @param order 阶数 @param cutoff 截止频率 @param sampleRate 采样率 @param rippleDb 通带纹波(dB) */
    void design(Type type, int order, double cutoff,
                double sampleRate, double rippleDb = 0.5);

    /** @brief 应用滤波 @param input 输入信号 @return 滤波后信号 */
    QVector<double> apply(const QVector<double>& input);

    QVector<double> numerator() const { return m_b; }
    QVector<double> denominator() const { return m_a; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterDesigned(int order, double ripple);
    void filterApplied(int inputSize);

private:
    /** @brief 切比雪夫多项式 T_n(x) */
    double chebyshevPoly(int n, double x) const;

    QVector<double> m_b;
    QVector<double> m_a;
    Stats m_stats;
    double m_timeSum;
};

#endif // CHEBYSHEVFILTER_H
