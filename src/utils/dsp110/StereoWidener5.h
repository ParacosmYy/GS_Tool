#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 立体声展宽处理器实现
 *
 * 通过中/侧(Mid/Side)处理和全通滤波器相移技术扩展立体声声场宽度，
 * 支持动态宽度控制和低频保护，适用于耳机虚拟化和混音后期处理。
 */
class StereoWidener5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit StereoWidener5(QObject* parent = nullptr);

    /** @brief 设置展宽宽度(0.0=单声道, 1.0=正常, 2.0=超宽) */
    void setWidth(double width);

    /** @brief 设置低频保护截止频率(Hz)，低频以下不展宽以保持居中 */
    void setLowFrequencyProtection(double freqHz);

    /** @brief 对立体声帧对执行展宽处理 */
    QVector<QPair<double, double>> process(const QVector<QPair<double, double>>& stereoFrames);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号，返回处理的帧数 */
    void processingCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_width = 1.5;
    double m_lpfFreq = 150.0;
};
