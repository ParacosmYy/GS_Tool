/**
 * @file PitchDetector.h
 * @brief 基频检测器(自相关+抛物线插值+有声/无声判决) — Pitch Detection via Autocorrelation with Parabolic Interpolation and Voiced/Unvoiced Decision
 *
 * 功能: 实现基于自相关的基频(F0)检测，支持抛物线插值提高频率精度，
 *       基于能量和过零率的有声/无声判决，以及倍频/半频校正。
 *
 * 协作: GranularSynthesis(颗粒合成) / FftEngine(频谱分析) / SignalFilter(滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 基频检测器
 */
class PitchDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 检测结果 */
    struct PitchResult {
        double frequency;           ///< 检测到的基频(Hz)，0表示无声
        double confidence;          ///< 置信度(0~1)
        bool voiced;                ///< 是否有声
        double energy;              ///< 帧能量
        double zeroCrossingRate;    ///< 过零率
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;            ///< 累计分析帧数
        quint64 voicedFrames = 0;           ///< 有声帧数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double avgFrequency = 0.0;          ///< 平均基频
    };

    explicit PitchDetector(QObject* parent = nullptr);
    ~PitchDetector() override;

    /** @brief 设置采样率 */
    void setSampleRate(int rate);

    /** @brief 设置最小基频(Hz) */
    void setMinFrequency(double freq);

    /** @brief 设置最大基频(Hz) */
    void setMaxFrequency(double freq);

    /** @brief 设置有声能量阈值 */
    void setVoicedThreshold(double threshold);

    /**
     * @brief 检测单帧基频
     * @param frame 音频帧
     * @return 检测结果
     */
    PitchResult detect(const QVector<double>& frame);

    /**
     * @brief 批量检测
     * @param signal 完整信号
     * @param frameSize 帧长(采样点)
     * @param hopSize 步长(采样点)
     * @return 逐帧检测结果
     */
    QVector<PitchResult> detectSequence(const QVector<double>& signal,
                                        int frameSize, int hopSize);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 单帧检测完成 @param freq 基频 @param voiced 是否有声 */
    void frameDetected(double freq, bool voiced);

private:
    /** @brief 计算自相关函数 */
    void autocorrelation(const QVector<double>& frame, QVector<double>& acf) const;

    /** @brief 抛物线插值精化峰值位置 */
    double parabolicInterpolation(const QVector<double>& acf, int peak) const;

    /** @brief 计算帧能量 */
    static double computeEnergy(const QVector<double>& frame);

    /** @brief 计算过零率 */
    static double computeZeroCrossingRate(const QVector<double>& frame);

    int m_sampleRate = 44100;
    double m_minFreq = 50.0;
    double m_maxFreq = 800.0;
    double m_voicedThreshold = 0.01;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_freqSum = 0.0;
};
