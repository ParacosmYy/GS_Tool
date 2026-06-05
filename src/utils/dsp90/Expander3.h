#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 动态扩展器工具类
 *
 * 提供信号动态范围扩展功能，支持设置阈值和扩展范围，
 * 用于增大低电平信号与高电平信号之间的差距。
 */
class Expander3 : public QObject {
    Q_OBJECT
public:
    /// 处理统计信息
    struct Stats {
        int totalProcessed = 0;     ///< 总处理帧数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Expander3(QObject* parent = nullptr);

    /** @brief 设置扩展阈值(dB) */
    void setThreshold(double thresholdDb);

    /** @brief 设置扩展范围(dB) */
    void setRange(double rangeDb);

    /** @brief 对输入信号帧执行动态扩展 */
    QVector<double> process(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号，返回输出帧数 */
    void processingCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = -40.0;
    double m_range = 20.0;
};
