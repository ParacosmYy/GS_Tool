#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief PhaseVocoder4 - 相位声码器
 *
 * 基于STFT的时域拉伸和变调处理，使用相位
 * 连续性校正实现高质量时间尺度修改。
 */
class PhaseVocoder4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalStretchOperations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PhaseVocoder4(QObject* parent = nullptr);

    /** @brief 设置STFT参数 */
    void initialize(int fftSize, int hopSize, double stretchFactor);

    /** @brief 时间拉伸(不改变音调) */
    QVector<double> timeStretch(const QVector<double>& input, double factor);

    /** @brief 变调(不改变时长) */
    QVector<double> pitchShift(const QVector<double>& input, double semitones);

    /** @brief 处理单帧并输出 */
    QVector<double> processFrame(const QVector<double>& frame);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(double phaseCorrection);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_fftSize = 2048;
    int m_hopSize = 512;
    double m_stretchFactor = 1.0;
    QVector<double> m_prevPhase;
};
