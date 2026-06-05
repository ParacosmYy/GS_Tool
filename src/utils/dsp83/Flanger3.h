#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Flanger音效处理器
 *
 * 短延迟线性调制，产生梳状滤波效果。
 */
class Flanger3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalBuffersApplied = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Flanger3(QObject* parent = nullptr);

    /** @brief 处理音频缓冲区 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 设置Flanger参数(延迟范围/速率/反馈) */
    void setParameters(double delayMs, double rateHz, double feedback);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void effectApplied(int sampleCount, double currentDelay);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_delay = 2.0;
    double m_rate = 0.5;
    double m_feedback = 0.7;
};
