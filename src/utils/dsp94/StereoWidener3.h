#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 立体声展宽处理器
 *
 * 通过调整左右声道间的相关性与延迟差来控制立体声宽度,
 * 支持从单声道兼容到超宽声场的连续调节,适用于混音后期处理。
 */
class StereoWidener3 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit StereoWidener3(QObject* parent = nullptr);

    /** @brief 设置立体声宽度(0.0=单声道, 1.0=正常, 2.0=超宽) */
    void setWidth(double width);

    /** @brief 处理双声道音频帧数据,输入格式: [left, right] */
    void process(const QVector<QVector<double>>& frames);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 处理完成信号 */
    void processingCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_width = 1.0;
};
