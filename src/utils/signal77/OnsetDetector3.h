#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief OnsetDetector3 - 音频起始点检测器
 *
 * 基于频谱通量和相位偏差的音乐起始点检测，
 * 支持多种检测函数和自适应阈值。
 */
class OnsetDetector3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalOnsetsDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit OnsetDetector3(QObject* parent = nullptr);

    /** @brief 设置采样率和帧大小 */
    void initialize(double sampleRate, int frameSize, int hopSize);

    /** @brief 处理一帧音频，返回起始点强度 */
    double processFrame(const QVector<double>& frame);

    /** @brief 检测整个音频文件的所有起始点位置(样本索引) */
    QVector<int> detectOnsets(const QVector<double>& audio);

    /** @brief 设置检测灵敏度(0.0~1.0) */
    void setSensitivity(double sensitivity);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void onsetDetected(int sampleIndex, double strength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
    double m_sensitivity = 0.5;
    int m_hopSize = 512;
};
