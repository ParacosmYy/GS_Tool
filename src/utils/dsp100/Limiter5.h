#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 音频限幅器
 *
 * 将信号绝对峰值限制在指定上限以内，防止削波失真，
 * 支持阈值、上限和释放时间参数调节。
 */
class Limiter5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalFrames = 0;         ///< 已处理帧数
        double avgGainReduction = 0.0; ///< 平均增益衰减量(dB)
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Limiter5(QObject* parent = nullptr);

    /** @brief 设置限幅阈值(dB) */
    void setThreshold(double thresholdDb);
    /** @brief 设置输出上限(dB) */
    void setCeiling(double ceilingDb);
    /** @brief 设置释放时间(ms) */
    void setRelease(double releaseMs);
    /** @brief 处理音频数据帧 */
    QVector<double> process(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成，返回输出帧数 */
    void processingCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = -3.0;
    double m_ceiling = -0.3;
    double m_release = 50.0;
};
