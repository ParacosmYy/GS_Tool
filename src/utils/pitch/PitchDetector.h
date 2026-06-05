/**
 * @file PitchDetector.h
 * @brief 基频检测器 — 自相关/YIN/AMDF算法
 *
 * 功能: 多种基频检测算法(自相关/YIN/AMDF)，统计检测次数/帧数/耗时。
 */
#ifndef PITCHDETECTOR_H
#define PITCHDETECTOR_H

#include <QObject>
#include <QVector>

class PitchDetector : public QObject {
    Q_OBJECT
public:
    enum class Method { Autocorrelation, Yin, Amdf };

    struct Result {
        double frequency = 0.0;
        double confidence = 0.0;
        bool voiced = false;
    };

    struct Stats {
        quint64 totalDetections = 0;
        quint64 totalFramesProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit PitchDetector(QObject* parent = nullptr);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 检测基频 @param data 信号帧 @param method 方法 @return 检测结果 */
    Result detect(const QVector<double>& data, Method method = Method::Yin);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(double frequency, double confidence);

private:
    Result detectAutocorrelation(const QVector<double>& data);
    Result detectYin(const QVector<double>& data);
    Result detectAmdf(const QVector<double>& data);

    double m_sampleRate;
    Stats m_stats;
    double m_timeSum;
};

#endif // PITCHDETECTOR_H
