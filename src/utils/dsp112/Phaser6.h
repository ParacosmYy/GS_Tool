#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 移相器(Phaser)效果处理器实现
 *
 * 通过级联全通滤波器组产生频率相关的相位偏移，与干信号混合产生
 * 梳状滤波效果的周期性频谱凹陷，支持LFO调制和多级级联。
 */
class Phaser6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit Phaser6(QObject* parent = nullptr);

    /** @brief 设置全通滤波器级联级数(2~12级)，级数越多凹陷越深 */
    void setStageCount(int stages);

    /** @brief 设置LFO调制速率(Hz)和深度(0.0~1.0) */
    void setLFOParams(double rateHz, double depth);

    /** @brief 设置干湿混合比(0.0=全干, 1.0=全湿) */
    void setMix(double mix);

    /** @brief 对输入音频帧执行移相处理 */
    QVector<double> process(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号，返回帧数 */
    void processingCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_stages = 4;
    double m_lfoRate = 0.5;
    double m_lfoDepth = 0.7;
    double m_mix = 0.5;
};
