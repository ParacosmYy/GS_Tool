/**
 * @file EnvelopeDetector.h
 * @brief 包络检测器 — Hilbert/峰值/对数包络
 *
 * 功能: 基于Hilbert变换/峰值保持/对数检波提取信号包络，
 *       统计检测次数/采样点数/耗时。
 */
#ifndef ENVELOPEDETECTOR_H
#define ENVELOPEDETECTOR_H

#include <QObject>
#include <QVector>

class EnvelopeDetector : public QObject {
    Q_OBJECT
public:
    enum class Method { Hilbert, PeakHold, LogDetect, Squelch };

    /** 检测统计 */
    struct Stats {
        quint64 totalDetections = 0;
        quint64 totalSamplesProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetector(QObject* parent = nullptr);

    /** @brief 检测包络 @param data 信号 @param method 方法 @return 包络 */
    QVector<double> detect(const QVector<double>& data, Method method);

    /** @brief Hilbert变换(解析信号虚部) @param data 信号 @return 虚部 */
    QVector<double> hilbertTransform(const QVector<double>& data) const;

    /** @brief 峰值保持包络 @param data 信号 @param holdTime 保持采样数 @param decay 衰减系数 @return 包络 */
    QVector<double> peakHoldEnvelope(const QVector<double>& data,
                                     int holdTime, double decay) const;

    /** @brief RMS包络 @param data 信号 @param windowSize 窗口大小 @return RMS包络 */
    QVector<double> rmsEnvelope(const QVector<double>& data, int windowSize) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(int sampleCount, Method method);

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // ENVELOPEDETECTOR_H
