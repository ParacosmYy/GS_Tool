/**
 * @file ButterworthFilter.h
 * @brief 巴特沃斯滤波器设计 — 最大平坦幅频响应
 *
 * 功能: 巴特沃斯IIR滤波器设计，支持低通/高通/带通/带阻，
 *       通带最大平坦特性，统计设计次数/应用次数/耗时。
 */
#ifndef BUTTERWORTHFILTER_H
#define BUTTERWORTHFILTER_H

#include <QObject>
#include <QVector>

class ButterworthFilter : public QObject {
    Q_OBJECT
public:
    enum Type { LowPass, HighPass, BandPass, BandStop };

    struct Stats {
        quint64 totalDesigns = 0;
        quint64 totalApplications = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit ButterworthFilter(QObject* parent = nullptr);

    /** @brief 设计低通/高通 @param type 类型 @param order 阶数 @param cutoff 截止频率 @param sampleRate 采样率 */
    void design(Type type, int order, double cutoff, double sampleRate);

    /** @brief 设计带通/带阻 @param type 类型 @param order 阶数 @param lowCutoff 低截止 @param highCutoff 高截止 @param sampleRate 采样率 */
    void designBand(Type type, int order, double lowCutoff,
                    double highCutoff, double sampleRate);

    /** @brief 应用滤波 @param input 输入信号 @return 滤波后信号 */
    QVector<double> apply(const QVector<double>& input);

    /** @brief 频率响应 @param freqs 频率数组 @return 幅度响应 */
    QVector<double> frequencyResponse(const QVector<double>& freqs) const;

    QVector<double> numerator() const { return m_b; }
    QVector<double> denominator() const { return m_a; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterDesigned(int order, double cutoff);
    void filterApplied(int inputSize);

private:
    QVector<double> m_b;
    QVector<double> m_a;
    int m_order;
    double m_sampleRate;
    Stats m_stats;
    double m_timeSum;
};

#endif // BUTTERWORTHFILTER_H
