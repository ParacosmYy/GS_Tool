#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 动态范围压缩器
 *
 * 对音频信号进行动态压缩，支持阈值、比率、拐点参数调节，
 * 用于音频处理链路中的动态范围控制。
 */
class Compressor5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalFrames = 0;         ///< 已处理帧数
        double avgGainReduction = 0.0; ///< 平均增益衰减量(dB)
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Compressor5(QObject* parent = nullptr);

    /** @brief 设置压缩阈值(dB) */
    void setThreshold(double thresholdDb);
    /** @brief 设置压缩比 */
    void setRatio(double ratio);
    /** @brief 设置软拐点宽度(dB) */
    void setKnee(double kneeDb);
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
    double m_threshold = -20.0;
    double m_ratio = 4.0;
    double m_knee = 6.0;
};
