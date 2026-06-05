#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Phaser音效处理器
 *
 * 全通滤波器级联产生相位抵消效果的处理器。
 */
class Phaser3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalStagesActive = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Phaser3(QObject* parent = nullptr);

    /** @brief 处理音频缓冲区 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 设置级数和调制参数 */
    void setParameters(int stages, double rateHz, double depth, double feedback);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void effectApplied(int sampleCount, int activeStages);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_stages = 4;
    double m_rate = 0.5;
};
