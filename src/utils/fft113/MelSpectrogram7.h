#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Mel频谱图(Mel Spectrogram)计算器实现
 *
 * 将线性频率轴的功率谱通过Mel滤波器组映射到Mel频率刻度，
 * 模拟人耳对频率的非线性感知，广泛应用于语音识别和音乐信息检索。
 */
class MelSpectrogram7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit MelSpectrogram7(QObject* parent = nullptr);

    /** @brief 设置采样率(Hz)和FFT窗口长度 */
    void setAudioParams(double sampleRate, int fftSize);

    /** @brief 设置Mel滤波器组数量，决定输出频率分辨率 */
    void setMelBinCount(int bins);

    /** @brief 设置频率范围(Hz)，限制Mel滤波器的有效频带 */
    void setFrequencyRange(double minFreq, double maxFreq);

    /** @brief 对输入音频信号计算Mel频谱图，返回每帧的Mel能量向量 */
    QVector<QVector<double>> compute(const QVector<double>& audio);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号，返回帧数和Mel频带数 */
    void computationCompleted(int frameCount, int melBins);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
    int m_fftSize = 2048;
    int m_melBins = 128;
    double m_minFreq = 0.0;
    double m_maxFreq = 22050.0;
};
