#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Mel频谱图计算器
 *
 * 将线性频率频谱通过Mel滤波器组映射到Mel刻度，
 * 广泛用于语音识别和音频分类的特征提取。
 */
class MelSpectrogram6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit MelSpectrogram6(QObject* parent = nullptr);

    /** @brief 设置采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置Mel频率箱数量 */
    void setBinCount(int bins);

    /** @brief 计算Mel频谱特征 */
    QVector<QVector<double>> compute(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void computationCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
    int m_binCount = 40;
};
