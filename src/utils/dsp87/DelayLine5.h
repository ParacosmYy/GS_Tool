#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 延迟线处理器
 *
 * 可变延迟的数字延迟线，支持反馈和交叉淡入淡出。
 */
class DelayLine5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalBuffersApplied = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DelayLine5(QObject* parent = nullptr);

    /** @brief 处理音频缓冲区 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 设置延迟参数(延迟时间/反馈/混合比) */
    void setParameters(double delayMs, double feedback, double mix);

    /** @brief 计算回声密度 */
    double echoDensity(double thresholdMin = -60.0) const;

    /** @brief 获取当前参数 */
    QVector<double> parameters() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void effectApplied(int sampleCount, double delayMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_delayMs = 250.0;
    double m_feedback = 0.3;
    double m_mix = 0.5;
};
