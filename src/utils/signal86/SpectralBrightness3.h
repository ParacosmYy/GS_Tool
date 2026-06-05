#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 频谱亮度分析器
 *
 * 计算音频频谱的亮度特征(高频能量占比)。
 */
class SpectralBrightness3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesAnalyzed = 0;
        int totalPeaksDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralBrightness3(QObject* parent = nullptr);

    /** @brief 计算单帧频谱亮度 */
    double compute(const QVector<double>& spectrum, double sampleRate, double cutoffHz = 4000.0);

    /** @brief 批量计算亮度序列 */
    QVector<double> computeSequence(const QVector<QVector<double>>& spectra,
                                     double sampleRate, double cutoffHz = 4000.0);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void brightnessComputed(double brightness, double cutoffHz);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_cutoffHz = 4000.0;
};
