/**
 * @file StereoEnhancer.h
 * @brief 立体声增强处理器 — Mid-Side Stereo Enhancement with Width Control
 *
 * 功能: 基于中间-两侧(M/S)处理的立体声增强器。支持可调节立体声宽度、
 *       中心提取、侧链增强和相位校正。适用于音频后处理和混音分析。
 *
 * 协作: BiquadFilter(双二阶滤波) / DynamicCompressor(动态压缩)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief M/S立体声增强处理器
 */
class StereoEnhancer : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;            ///< 累计处理帧数
        quint64 totalSamples = 0;           ///< 累计采样点数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double peakMid = 0.0;               ///< Mid通道峰值
        double peakSide = 0.0;              ///< Side通道峰值
    };

    explicit StereoEnhancer(QObject* parent = nullptr);

    /**
     * @brief 设置立体声宽度(0.0=单声道, 1.0=原始, 2.0=超宽)
     * @param width 宽度系数
     */
    void setWidth(double width);

    /**
     * @brief 设置中心电平
     * @param level 中心增益(0.0-2.0)
     */
    void setCenterLevel(double level);

    /**
     * @brief 设置两侧电平
     * @param level 侧链增益(0.0-2.0)
     */
    void setSideLevel(double level);

    /**
     * @brief 启用/禁用相位校正
     */
    void setPhaseCorrection(bool enable);

    /**
     * @brief 处理立体声帧(L/R对)
     * @param left 左声道
     * @param right 右声道
     * @return 增强后的(L,R)对
     */
    QPair<QVector<double>, QVector<double>> process(
        const QVector<double>& left, const QVector<double>& right);

    /**
     * @brief 提取Mid信号
     */
    QVector<double> extractMid(const QVector<double>& left,
                                const QVector<double>& right) const;

    /**
     * @brief 提取Side信号
     */
    QVector<double> extractSide(const QVector<double>& left,
                                 const QVector<double>& right) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param frames 帧数 @param peakMid Mid峰值 */
    void processCompleted(int frames, double peakMid);

private:
    /** @brief 相位校正：检测并修正反相 */
    double correctPhase(double mid, double side) const;

    double m_width = 1.0;
    double m_centerLevel = 1.0;
    double m_sideLevel = 1.0;
    bool m_phaseCorrection = false;

    Stats m_stats;
    double m_timeSum = 0.0;
};
