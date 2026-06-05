#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 限幅器工具类
 *
 * 提供信号限幅功能，支持设置阈值和上限，
 * 确保输出信号不超过指定的最大电平。
 */
class Limiter4 : public QObject {
    Q_OBJECT
public:
    /// 处理统计信息
    struct Stats {
        int totalProcessed = 0;     ///< 总处理帧数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Limiter4(QObject* parent = nullptr);

    /** @brief 设置限幅阈值(dB) */
    void setThreshold(double thresholdDb);

    /** @brief 设置输出上限(dB) */
    void setCeiling(double ceilingDb);

    /** @brief 对输入信号帧执行限幅处理 */
    QVector<double> process(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号，返回输出帧数 */
    void processingCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = -6.0;
    double m_ceiling = -0.3;
};
