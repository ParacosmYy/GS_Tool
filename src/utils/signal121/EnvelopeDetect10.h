#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief EnvelopeDetect10 - 包络检测第10代实现
 *
 * 提取音频信号的幅度包络，支持Hilbert变换法、
 * 峰值检测法、RMS包络及可调平滑时间。
 */
class EnvelopeDetect10 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetectOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit EnvelopeDetect10(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 使用Hilbert变换提取信号包络
     * @param signal 输入音频信号
     * @return 包络幅度序列
     */
    QVector<double> hilbertEnvelope(const QVector<double>& signal);

    /**
     * @brief 使用峰值检测法提取包络
     * @param signal 输入信号
     * @param attackMs 攻击时间 (ms)
     * @param releaseMs 释放时间 (ms)
     * @param sampleRate 采样率 (Hz)
     * @return 包络序列
     */
    QVector<double> peakEnvelope(const QVector<double>& signal,
                                 double attackMs, double releaseMs,
                                 double sampleRate);

    /**
     * @brief 使用RMS方法提取包络
     * @param signal 输入信号
     * @param windowSize RMS窗口大小（采样点数）
     * @return RMS包络序列
     */
    QVector<double> rmsEnvelope(const QVector<double>& signal, int windowSize);

    /**
     * @brief 设置包络平滑滤波器截止频率
     * @param cutoffHz 截止频率 (Hz)
     * @param sampleRate 采样率 (Hz)
     */
    void setSmoothing(double cutoffHz, double sampleRate);

signals:
    void detectionCompleted(int sampleCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
