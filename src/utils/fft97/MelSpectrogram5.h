#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Mel频谱图计算器
 *
 * 将线性频率频谱通过Mel滤波器组映射到感知均匀的Mel频率轴,
 * 广泛应用于语音识别、音频分类与音乐生成的特征提取前端。
 */
class MelSpectrogram5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit MelSpectrogram5(QObject* parent = nullptr);

    /** @brief 设置采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置Mel频率箱数 */
    void setBinCount(int bins);

    /** @brief 计算Mel频谱图特征 */
    void compute(const QVector<double>& samples);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号,返回Mel频率箱数 */
    void computationCompleted(int binCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
    int m_binCount = 128;
};
