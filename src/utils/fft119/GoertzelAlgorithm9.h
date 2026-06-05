#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief GoertzelAlgorithm9 - Goertzel算法第9代实现
 *
 * 高效计算单个频点的DFT值，适用于DTMF检测、
 * 音调识别等只需少量频点的场景，复杂度O(N)。
 */
class GoertzelAlgorithm9 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; double avgProcessingTimeMs = 0.0; };
    explicit GoertzelAlgorithm9(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 计算指定频率的DFT值
     * @param signal 输入时域信号
     * @param targetFreqHz 目标频率 (Hz)
     * @param sampleRate 采样率 (Hz)
     * @return 复数DFT值 (实部, 虚部)
     */
    QPair<double, double> compute(const QVector<double>& signal,
                                  double targetFreqHz, double sampleRate);

    /**
     * @brief 批量计算多个目标频率的DFT值
     * @param signal 输入信号
     * @param targetFreqs 目标频率列表 (Hz)
     * @param sampleRate 采样率 (Hz)
     * @return 各频率的复数DFT值
     */
    QVector<QPair<double, double>> computeMultiFreq(
        const QVector<double>& signal,
        const QVector<double>& targetFreqs, double sampleRate);

    /**
     * @brief 计算指定频率的功率
     * @param signal 输入信号
     * @param targetFreqHz 目标频率 (Hz)
     * @param sampleRate 采样率 (Hz)
     * @return 功率值
     */
    double computePower(const QVector<double>& signal,
                        double targetFreqHz, double sampleRate);

    /**
     * @brief 检测信号中是否存在指定频率（基于阈值）
     * @param signal 输入信号
     * @param targetFreqHz 目标频率 (Hz)
     * @param sampleRate 采样率 (Hz)
     * @param thresholdDb 检测阈值 (dB)
     * @return 是否检测到该频率
     */
    bool detectTone(const QVector<double>& signal, double targetFreqHz,
                    double sampleRate, double thresholdDb = -30.0);

signals:
    void computationCompleted(int frequencyCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
