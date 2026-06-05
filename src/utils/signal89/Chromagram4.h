#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 色度图(Chromagram)分析器
 *
 * 将频谱映射到12个音高类别(C~B)，用于和弦和调性分析。
 */
class Chromagram4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesComputed = 0;
        int totalChordsDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chromagram4(QObject* parent = nullptr);

    /** @brief 计算单帧色度向量(12维) */
    QVector<double> compute(const QVector<double>& spectrum, double sampleRate);

    /** @brief 批量计算色度图序列 */
    QVector<QVector<double>> computeSequence(const QVector<double>& samples,
                                              double sampleRate, int frameSize, int hopSize);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void chromagramComputed(int frameIndex, int dominantPitchClass);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
