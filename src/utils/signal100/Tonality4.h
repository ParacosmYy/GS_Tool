#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 音调性分析器
 *
 * 分析信号的音调特性，通过自相关和频谱峰值检测
 * 评估信号的音调强度和基频。
 */
class Tonality4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalFrames = 0;         ///< 已处理帧数
        double avgTonality = 0.0;    ///< 平均音调性
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Tonality4(QObject* parent = nullptr);

    /** @brief 设置分析帧大小 */
    void setFrameSize(int size);
    /** @brief 设置采样率(Hz) */
    void setSampleRate(double rate);
    /** @brief 计算音调性指标 */
    double compute(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成，返回音调性值 */
    void computed(double tonality);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_frameSize = 2048;
    double m_sampleRate = 44100.0;
};
