#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 立体声展宽效果处理器
 *
 * 通过调节左右声道间的差异量和相关性来实现立体声像的展宽，
 * 支持从单声道到超宽立体声的连续调节。
 */
class StereoWidener4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit StereoWidener4(QObject* parent = nullptr);

    /** @brief 设置立体声宽度(0.0~2.0) */
    void setWidth(double width);

    /** @brief 处理立体声采样数据[left, right] */
    QVector<QVector<double>> process(const QVector<QVector<double>>& frames);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号 */
    void processingCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_width = 1.0;
};
