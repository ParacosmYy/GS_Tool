#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief MFCC特征提取器
 *
 * Mel频率倒谱系数提取，用于语音识别和音频分类。
 */
class MFCC4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesExtracted = 0;
        int totalCoefficients = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MFCC4(QObject* parent = nullptr);

    /** @brief 提取单帧MFCC系数 */
    QVector<double> extract(const QVector<double>& frame, double sampleRate, int numCoeffs = 13);

    /** @brief 批量提取MFCC特征序列 */
    QVector<QVector<double>> extractSequence(const QVector<double>& samples,
                                              double sampleRate, int frameSize, int hopSize);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameExtracted(int frameIndex, int coeffCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_numCoeffs = 13;
};
