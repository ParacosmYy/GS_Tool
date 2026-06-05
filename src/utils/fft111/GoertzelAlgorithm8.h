#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Goertzel算法实现
 *
 * 高效计算指定频率点的DFT值，无需执行完整FFT运算，
 * 适用于DTMF检测、音调频率识别和窄带频谱分析等只需少量频率点的场景。
 */
class GoertzelAlgorithm8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit GoertzelAlgorithm8(QObject* parent = nullptr);

    /** @brief 设置目标检测频率列表(Hz) */
    void setTargetFrequencies(const QVector<double>& frequencies);

    /** @brief 设置采样率(Hz)，用于将频率映射到DFT系数 */
    void setSampleRate(double sampleRate);

    /** @brief 对输入信号计算所有目标频率的幅值和相位 */
    QVector<QPair<double, double>> compute(const QVector<double>& input);

    /** @brief 单频点快速检测，返回幅值(用于DTMF等实时场景) */
    double computeSingle(const QVector<double>& input, double targetFreq);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号，返回检测到的频率点数 */
    void computationCompleted(int frequencyCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
    QVector<double> m_targetFreqs;
};
