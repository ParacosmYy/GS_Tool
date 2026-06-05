#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 频谱通量计算器
 *
 * 计算连续频谱帧之间的通量变化，用于onset检测和音频分段。
 */
class SpectralFlux3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalOnsetsDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralFlux3(QObject* parent = nullptr);

    /** @brief 计算频谱通量序列 */
    QVector<double> compute(const QVector<QVector<double>>& spectra);

    /** @brief 基于通量检测onset点 */
    QVector<int> detectOnsets(const QVector<double>& flux, double threshold);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void onsetDetected(int frameIndex, double fluxValue);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_prevSpectrum;
};
