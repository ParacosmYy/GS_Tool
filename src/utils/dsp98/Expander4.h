#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 动态范围扩展器
 *
 * 低于阈值的信号被进一步衰减，增加动态范围，
 * 支持起音时间、扩展范围等参数调节。
 */
class Expander4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalFrames = 0;         ///< 已处理帧数
        double avgGainReduction = 0.0; ///< 平均增益衰减量(dB)
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Expander4(QObject* parent = nullptr);

    /** @brief 设置扩展阈值(dB) */
    void setThreshold(double thresholdDb);
    /** @brief 设置扩展范围(dB) */
    void setRange(double rangeDb);
    /** @brief 设置起音时间(ms) */
    void setAttack(double attackMs);
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
    double m_threshold = -40.0;
    double m_range = -60.0;
    double m_attack = 10.0;
};
