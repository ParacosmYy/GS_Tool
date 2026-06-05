#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief TransientDetect6 - 瞬态检测第6代实现
 *
 * 检测音频信号中的瞬态事件（打击声、辅音起声等），
 * 支持基于能量导数、频谱通量及多特征融合的检测方法。
 */
class TransientDetect6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetectOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit TransientDetect6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 检测信号中的瞬态位置
     * @param signal 输入音频信号
     * @param threshold 检测阈值 [0, 1]
     * @return 瞬态发生位置的采样点索引
     */
    QVector<int> detect(const QVector<double>& signal, double threshold = 0.5);

    /**
     * @brief 计算频谱通量特征（帧间频谱差异）
     * @param frames 分帧后的频谱序列
     * @return 各帧的频谱通量值
     */
    QVector<double> spectralFlux(const QVector<QVector<double>>& frames);

    /**
     * @brief 计算能量导数特征
     * @param signal 输入信号
     * @param hopSize 帧移
     * @return 能量导数序列
     */
    QVector<double> energyDerivative(const QVector<double>& signal, int hopSize = 256);

    /**
     * @brief 设置检测灵敏度与最小瞬态间隔
     * @param sensitivity 灵敏度 (low/medium/high)
     * @param minIntervalMs 最小瞬态间隔 (ms)
     * @param sampleRate 采样率 (Hz)
     */
    void setDetectionParams(const QString& sensitivity, double minIntervalMs,
                            double sampleRate);

signals:
    void detectionCompleted(int transientCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
