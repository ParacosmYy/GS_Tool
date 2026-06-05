#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 动态均衡器
 *
 * 根据输入信号强度自动调整频段增益的智能均衡器。
 */
class DynamicEQ4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalBandAdjustments = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DynamicEQ4(QObject* parent = nullptr);

    /** @brief 处理音频帧 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 设置频段参数(中心频率/阈值/增益范围) */
    void setBand(int bandIndex, double freqHz, double thresholdDb, double ratio);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void bandAdjusted(int bandIndex, double gainDb);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_bandCount = 4;
};
