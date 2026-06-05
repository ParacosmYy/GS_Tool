#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 噪声门效果器工具类
 *
 * 提供噪声门处理功能，当信号低于阈值时自动衰减，
 * 支持设置阈值和释放时间。
 */
class Gate3 : public QObject {
    Q_OBJECT
public:
    /// 处理统计信息
    struct Stats {
        int totalProcessed = 0;     ///< 总处理帧数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Gate3(QObject* parent = nullptr);

    /** @brief 设置门限阈值(dB) */
    void setThreshold(double thresholdDb);

    /** @brief 设置释放时间(ms) */
    void setRelease(double releaseMs);

    /** @brief 对输入信号帧执行噪声门处理 */
    QVector<double> process(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号，返回输出帧数 */
    void processingCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = -30.0;
    double m_release = 100.0;
};
