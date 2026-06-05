#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Goertzel算法实现
 *
 * 高效计算单个目标频率的DFT分量，适用于DTMF检测等
 * 只需特定频率分量的场景，复杂度O(N)优于完整FFT。
 */
class GoertzelAlgorithm7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit GoertzelAlgorithm7(QObject* parent = nullptr);

    /** @brief 设置目标检测频率(Hz) */
    void setTargetFreq(double freq);

    /** @brief 设置采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 计算目标频率的幅度和相位 */
    QPair<double,double> compute(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号，返回目标频率幅度 */
    void computationCompleted(double magnitude);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_targetFreq = 1000.0;
    double m_sampleRate = 44100.0;
};
