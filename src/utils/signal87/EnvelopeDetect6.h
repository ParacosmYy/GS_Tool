#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器
 *
 * 基于Hilbert变换或峰值保持的信号包络提取。
 */
class EnvelopeDetect6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalPeaksDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetect6(QObject* parent = nullptr);

    /** @brief 基于Hilbert变换提取包络 */
    QVector<double> hilbertEnvelope(const QVector<double>& signal);

    /** @brief 基于峰值保持提取包络 */
    QVector<double> peakEnvelope(const QVector<double>& signal, int holdSamples = 64);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void envelopeExtracted(int sampleCount, double peakAmplitude);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_holdSamples = 64;
};
