#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 常数Q变换(Constant-Q Transform, CQT)实现
 *
 * 在对数频率轴上等间隔分布频率箱，低频高分辨率、高频低分辨率，
 * 模拟人耳对音高的感知特性，适用于音乐分析和音高追踪。
 */
class ConstantQ7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalTransforms = 0; double avgProcessingTimeMs = 0.0; };

    explicit ConstantQ7(QObject* parent = nullptr);

    /** @brief 设置最低频率和最高频率(Hz)，定义分析频带范围 */
    void setFrequencyRange(double minFreq, double maxFreq);

    /** @brief 设置每八度的频率箱数(BPO)，值越大频率分辨率越高 */
    void setBinsPerOctave(int bpo);

    /** @brief 对输入信号执行CQT变换，返回复数频谱系数 */
    QVector<QPair<double, double>> transform(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成信号，返回频率箱数 */
    void transformCompleted(int binCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_minFreq = 32.7;   // C1
    double m_maxFreq = 4186.0; // C8
    int m_binsPerOctave = 12;
};
