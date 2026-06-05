#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 过零率计算器
 *
 * 计算信号的过零率，用于语音/音频活动检测和分类。
 */
class ZeroCrossing4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesAnalyzed = 0;
        int totalCrossings = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZeroCrossing4(QObject* parent = nullptr);

    /** @brief 计算单帧过零率 */
    double compute(const QVector<double>& frame) const;

    /** @brief 批量计算过零率序列 */
    QVector<double> computeSequence(const QVector<double>& samples, int frameSize, int hopSize);

    /** @brief 利用ZCR估计频率 */
    double estimateFrequency(const QVector<double>& frame, double sampleRate) const;

    /** @brief 检测帧是否为有声 */
    bool isVoiced(const QVector<double>& frame, double lowThreshold = 0.01, double highThreshold = 0.45) const;

    /** @brief 计算短时能量 */
    double shortTimeEnergy(const QVector<double>& frame) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void zeroCrossingRateComputed(double rate, int frameIndex);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
