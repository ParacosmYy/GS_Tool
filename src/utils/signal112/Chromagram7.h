#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 色度图(Chromagram)分析器实现
 *
 * 将音频频谱映射到12个音级(Pitch Class)维度，忽略八度差异，
 * 适用于和弦识别、音乐特征提取和音高内容分析。
 */
class Chromagram7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalAnalyzed = 0; double avgProcessingTimeMs = 0.0; };

    explicit Chromagram7(QObject* parent = nullptr);

    /** @brief 设置参考频率A4(Hz)，默认440Hz */
    void setReferenceFrequency(double freqHz);

    /** @brief 设置分析窗口长度和跳步大小(样本数) */
    void setFrameParams(int frameSize, int hopSize);

    /** @brief 对音频信号计算色度特征序列，返回12维向量列表 */
    QVector<QVector<double>> compute(const QVector<double>& audio);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分析完成信号，返回色度帧数 */
    void analysisCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_refFreq = 440.0;
    int m_frameSize = 4096;
    int m_hopSize = 2048;
};
