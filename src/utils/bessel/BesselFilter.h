/**
 * @file BesselFilter.h
 * @brief 贝塞尔滤波器设计 — 最大平坦群延迟
 *
 * 功能: 贝塞尔(Bessel)IIR滤波器设计，支持低通/高通/带通/带阻，
 *       最大平坦群延迟特性，统计设计次数/耗时。
 */
#ifndef BESSELFILTER_H
#define BESSELFILTER_H

#include <QObject>
#include <QVector>

class BesselFilter : public QObject {
    Q_OBJECT
public:
    /** 滤波器类型 */
    enum Type { LowPass, HighPass, BandPass, BandStop };

    /** 统计 */
    struct Stats {
        quint64 totalDesigns = 0;
        quint64 totalApplications = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit BesselFilter(QObject* parent = nullptr);

    /** @brief 设计滤波器 @param type 类型 @param order 阶数 @param cutoff 截止频率(Hz) @param sampleRate 采样率(Hz) */
    void design(Type type, int order, double cutoff, double sampleRate);

    /** @brief 设计带通/带阻 @param type 类型 @param order 阶数 @param lowCutoff 低截止 @param highCutoff 高截止 @param sampleRate 采样率 */
    void designBand(Type type, int order, double lowCutoff,
                    double highCutoff, double sampleRate);

    /** @brief 应用滤波器 @param input 输入信号 @return 滤波后信号 */
    QVector<double> apply(const QVector<double>& input);

    /** @brief 获取分子系数 */
    QVector<double> numerator() const { return m_b; }
    /** @brief 获取分母系数 */
    QVector<double> denominator() const { return m_a; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterDesigned(int order, double cutoff);
    void filterApplied(int inputSize);

private:
    /** @brief 贝塞尔多项式系数 */
    QVector<double> besselPolynomial(int n) const;
    /** @brief 双线性变换 */
    void bilinearTransform(QVector<double>& analogB,
                           QVector<double>& analogA,
                           double sampleRate);

    QVector<double> m_b;
    QVector<double> m_a;
    Stats m_stats;
    double m_timeSum;
};

#endif // BESSELFILTER_H
