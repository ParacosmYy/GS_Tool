#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Chromagram3 - 色度图(Chromagram)分析器
 *
 * 将频谱映射到12个色度(音高类)上，用于音乐
 * 和弦识别、音调分析和音乐信息检索。
 */
class Chromagram3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesAnalyzed = 0;
        int totalChromaChanges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chromagram3(QObject* parent = nullptr);

    /** @brief 设置采样率和参考频率(A4) */
    void initialize(double sampleRate, double refFreq = 440.0);

    /** @brief 计算一帧的12维色度向量 */
    QVector<double> computeChroma(const QVector<double>& frame);

    /** @brief 识别当前帧的主和弦 */
    QString detectChord(const QVector<double>& chroma) const;

    /** @brief 获取色度向量中的主导音高类索引(0=C, 1=C#...) */
    int dominantPitchClass(const QVector<double>& chroma) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void chromaComputed(const QVector<double>& chroma, QString chord);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
    double m_refFreq = 440.0;
};
