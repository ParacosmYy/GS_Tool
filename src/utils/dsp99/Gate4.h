#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 噪声门处理器
 *
 * 当信号低于阈值时将其衰减至静音，用于去除背景噪声，
 * 支持起音和释放时间参数调节。
 */
class Gate4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalFrames = 0;         ///< 已处理帧数
        double avgGainReduction = 0.0; ///< 平均增益衰减量(dB)
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Gate4(QObject* parent = nullptr);

    /** @brief 设置门限阈值(dB) */
    void setThreshold(double thresholdDb);
    /** @brief 设置起音时间(ms) */
    void setAttack(double attackMs);
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
    double m_threshold = -50.0;
    double m_attack = 1.0;
    double m_release = 100.0;
};
