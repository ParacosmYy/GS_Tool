/**
 * @file PolyphaseResampler.h
 * @brief 多相重采样器 — 任意比率采样率转换
 *
 * 功能: 多相滤波器实现任意比率采样率转换，
 *       支持上采样/下采样，统计处理次数/耗时。
 */
#ifndef POLYPHASERESAMPLER_H
#define POLYPHASERESAMPLER_H

#include <QObject>
#include <QVector>

class PolyphaseResampler : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalResamples = 0;
        quint64 totalSamplesIn = 0;
        quint64 totalSamplesOut = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit PolyphaseResampler(QObject* parent = nullptr);

    /** @brief 配置重采样比率 @param inRate 输入采样率 @param outRate 输出采样率 @param taps 滤波器抽头数 */
    void configure(double inRate, double outRate, int taps = 64);

    /** @brief 执行重采样 @param input 输入信号 @return 重采样后信号 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 当前比率 */
    double ratio() const { return m_ratio; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resampleCompleted(int inputSize, int outputSize);

private:
    double m_ratio;
    int m_upFactor;
    int m_downFactor;
    QVector<double> m_filter;
    int m_taps;
    Stats m_stats;
    double m_timeSum;
};

#endif // POLYPHASERESAMPLER_H
