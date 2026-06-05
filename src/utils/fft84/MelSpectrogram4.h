#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Mel频谱图计算器
 *
 * 将线性频谱转换为Mel刻度频谱，广泛用于语音和音频特征提取。
 */
class MelSpectrogram4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesComputed = 0;
        int totalMelBins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MelSpectrogram4(QObject* parent = nullptr);

    /** @brief 计算Mel频谱图 */
    QVector<QVector<double>> compute(const QVector<double>& samples, double sampleRate,
                                      int fftSize, int melBands = 40);

    /** @brief 获取Mel滤波器组 */
    QVector<QVector<double>> melFilterBank(int fftSize, int melBands, double sampleRate) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrogramComputed(int frameCount, int melBands);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_melBands = 40;
};
