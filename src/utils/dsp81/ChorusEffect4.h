#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 合唱音效处理器
 *
 * 多声部延迟调制，产生丰富的合唱/空间感效果。
 */
class ChorusEffect4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalBuffersApplied = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChorusEffect4(QObject* parent = nullptr);

    /** @brief 处理音频缓冲区 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 设置调制深度和速率 */
    void setModulation(double depth, double rateHz);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void effectApplied(int sampleCount, double modDepth);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_depth = 0.5;
    double m_rate = 1.5;
};
