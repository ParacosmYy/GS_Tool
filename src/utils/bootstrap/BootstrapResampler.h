/**
 * @file BootstrapResampler.h
 * @brief Bootstrap重采样 — 统计推断/置信区间
 *
 * 功能: 非参数Bootstrap重采样，计算均值/中位数/任意统计量的置信区间，
 *       统计重采样次数/样本数/耗时。
 */
#ifndef BOOTSTRAPRESAMPLER_H
#define BOOTSTRAPRESAMPLER_H

#include <QObject>
#include <QVector>
#include <functional>

class BootstrapResampler : public QObject {
    Q_OBJECT
public:
    struct ConfidenceInterval {
        double lower = 0.0;
        double upper = 0.0;
        double estimate = 0.0;
        double level = 0.95;
    };

    struct Stats {
        quint64 totalResamples = 0;
        quint64 totalIterations = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit BootstrapResampler(QObject* parent = nullptr);

    using StatFunc = std::function<double(const QVector<double>&)>;

    /** @brief Bootstrap置信区间 @param data 原始数据 @param statFunc 统计量函数 @param iterations 重采样次数 @param confidence 置信水平 @return 置信区间 */
    ConfidenceInterval computeCI(const QVector<double>& data,
                                  StatFunc statFunc,
                                  int iterations = 1000,
                                  double confidence = 0.95);

    /** @brief Bootstrap均值 @param data 数据 @param iterations 次数 @param confidence 置信水平 @return 置信区间 */
    ConfidenceInterval meanCI(const QVector<double>& data,
                               int iterations = 1000,
                               double confidence = 0.95);

    /** @brief Bootstrap中位数 @param data 数据 @param iterations 次数 @param confidence 置信水平 @return 置信区间 */
    ConfidenceInterval medianCI(const QVector<double>& data,
                                 int iterations = 1000,
                                 double confidence = 0.95);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resampleCompleted(int iteration, double currentValue);

private:
    QVector<double> resample(const QVector<double>& data) const;

    Stats m_stats;
    double m_timeSum;
};

#endif // BOOTSTRAPRESAMPLER_H
