#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 滑动DFT处理器
 *
 * 逐采样更新的DFT计算，适用于实时频谱分析。
 */
class SlidingDFT4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalBinsUpdated = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SlidingDFT4(QObject* parent = nullptr);

    /** @brief 初始化滑动DFT(指定bin数) */
    void init(int binCount, double sampleRate);

    /** @brief 输入单个采样值，更新所有bin */
    void pushSample(double sample);

    /** @brief 获取当前所有bin的幅值 */
    QVector<double> magnitudes() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumUpdated(int binCount, double dominantFreq);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_binCount = 0;
};
