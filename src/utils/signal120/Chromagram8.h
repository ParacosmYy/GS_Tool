#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Chromagram8 - 色度图分析第8代实现
 *
 * 将音频频谱映射到12个音级（C, C#, ..., B）的能量分布，
 * 广泛用于和弦识别、音乐结构分析及音调检测。
 */
class Chromagram8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalAnalysisOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit Chromagram8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 计算单帧色度向量
     * @param frequencySpectrum 频域幅度谱
     * @param sampleRate 采样率 (Hz)
     * @return 12维色度向量 (C~B)
     */
    QVector<double> compute(const QVector<double>& frequencySpectrum, double sampleRate);

    /**
     * @brief 批量计算色度图序列
     * @param frames 分帧后的频谱序列
     * @param sampleRate 采样率 (Hz)
     * @return 色度图矩阵 (帧数 × 12)
     */
    QVector<QVector<double>> computeSequence(
        const QVector<QVector<double>>& frames, double sampleRate);

    /**
     * @brief 设置参考频率（A4音高）
     * @param refFreqHz 参考频率 (Hz)，默认440Hz
     */
    void setReferenceFrequency(double refFreqHz);

    /**
     * @brief 识别当前帧的主和弦
     * @param chroma 色度向量
     * @return 最匹配的和弦名称
     */
    QString identifyChord(const QVector<double>& chroma) const;

signals:
    void analysisCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
