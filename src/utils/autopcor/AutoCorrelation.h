/**
 * @file AutoCorrelation.h
 * @brief 自相关分析器 — 周期性检测与基频估计
 *
 * 功能: 计算信号自相关函数，支持归一化/循环自相关，
 *       周期检测/基频估计，统计分析次数/耗时。
 */
#ifndef AUTOCORRELATION_H
#define AUTOCORRELATION_H

#include <QObject>
#include <QVector>

class AutoCorrelation : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalLagsProcessed = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    /** 分析结果 */
    struct AcfResult {
        QVector<double> acf;           ///< 自相关值
        int fundamentalLag = 0;        ///< 基频对应的lag
        double fundamentalFreq = 0.0;  ///< 估算基频
        double periodicity = 0.0;      ///< 周期性强度[0,1]
    };

    explicit AutoCorrelation(QObject* parent = nullptr);

    /** @brief 计算自相关 @param data 信号 @param maxLag 最大延迟 @return ACF结果 */
    AcfResult compute(const QVector<double>& data, int maxLag = 0);

    /** @brief 归一化自相关 @param data 信号 @param maxLag 最大延迟 @return 归一化ACF */
    QVector<double> computeNormalized(const QVector<double>& data,
                                       int maxLag = 0);

    /** @brief 检测周期性 @param data 信号 @param sampleRate 采样率 @return 周期性强度[0,1] */
    double detectPeriodicity(const QVector<double>& data, double sampleRate = 1.0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int lag, double freq);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // AUTOCORRELATION_H
