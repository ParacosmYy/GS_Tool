/**
 * @file AutomaticGainControl.h
 * @brief 自动增益控制 — 信号电平自动调节
 *
 * 功能: 基于反馈环路的AGC，支持目标电平/攻击时间/释放时间配置，
 *       统计处理样本数/增益调整次数/耗时。
 */
#ifndef AUTOMATICGAINCONTROL_H
#define AUTOMATICGAINCONTROL_H

#include <QObject>
#include <QVector>

class AutomaticGainControl : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalSamplesProcessed = 0;
        quint64 totalGainAdjustments = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit AutomaticGainControl(QObject* parent = nullptr);

    void setTargetLevel(double level);
    void setAttackTime(double ms);
    void setReleaseTime(double ms);
    void setMaxGain(double maxGain);

    /** @brief 处理单样本 @param sample 输入 @return 增益调节后输出 */
    double process(double sample);

    /** @brief 批量处理 @param data 输入信号 @return AGC处理后信号 */
    QVector<double> processBatch(const QVector<double>& data);

    double currentGain() const { return m_gain; }
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void gainChanged(double newGain);

private:
    double m_targetLevel;
    double m_attackCoeff;
    double m_releaseCoeff;
    double m_maxGain;
    double m_gain;
    Stats m_stats;
    double m_timeSum;
};

#endif // AUTOMATICGAINCONTROL_H
