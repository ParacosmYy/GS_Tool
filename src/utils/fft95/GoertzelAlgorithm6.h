#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Goertzel算法实现
 *
 * 高效计算离散傅里叶变换在单个指定频率上的值,无需完整FFT,
 * 适用于DTMF检测、音调识别与窄带频率分量分析。
 */
class GoertzelAlgorithm6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit GoertzelAlgorithm6(QObject* parent = nullptr);

    /** @brief 设置目标检测频率(Hz) */
    void setTargetFreq(double freq);

    /** @brief 设置采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 对采样数据执行Goertzel计算 */
    void compute(const QVector<double>& samples);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号,返回目标频率的幅值 */
    void computationCompleted(double magnitude);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_targetFreq = 1000.0;
    double m_sampleRate = 44100.0;
};
