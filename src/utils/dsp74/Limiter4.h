#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Limiter4 - 音频峰值限制器
 *
 * 带前瞻缓冲的砖墙限制器，支持瞬态保留和
 * 自适应释放时间，防止信号超过阈值。
 */
class Limiter4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalGainReductions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter4(QObject* parent = nullptr);

    /** @brief 设置限制阈值(dB) */
    void setThreshold(double thresholdDb);

    /** @brief 设置释放时间(ms) */
    void setReleaseTime(double releaseMs);

    /** @brief 处理音频帧，返回增益控制后的输出 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 获取当前增益 reduction(dB) */
    double currentGainReduction() const;

    /** @brief 设置前瞻时间(ms) */
    void setLookahead(double lookaheadMs);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void gainReductionApplied(double reductionDb);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = 0.0;
    double m_release = 50.0;
    double m_gainReduction = 0.0;
};
