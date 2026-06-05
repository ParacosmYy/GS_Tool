#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Tonality6 - 音调性分析第6代实现
 *
 * 计算音频信号的音调性指标，区分音调成分与噪声成分，
 * 支持自相关法、频谱峰值法及主观音调性评分。
 */
class Tonality6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalAnalysisOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit Tonality6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 计算音调性指标
     * @param signal 输入音频信号
     * @return 音调性值 [0, 1]，1为纯音调
     */
    double compute(const QVector<double>& signal);

    /**
     * @brief 检测信号中的音调频率
     * @param signal 输入音频信号
     * @param sampleRate 采样率 (Hz)
     * @return 检测到的音调频率列表 (Hz)
     */
    QVector<double> detectTonalFrequencies(const QVector<double>& signal, double sampleRate);

    /**
     * @brief 计算自相关函数用于音调检测
     * @param signal 输入信号
     * @param maxLag 最大延迟点数
     * @return 自相关值序列
     */
    QVector<double> autocorrelation(const QVector<double>& signal, int maxLag);

signals:
    void analysisCompleted(int tonalComponentCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
