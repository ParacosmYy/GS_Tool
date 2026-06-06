/**
 * @file SignalGenerator2.h
 * @brief 多波形信号发生器(AM/FM/PM调制+任意波形) — Multi-Waveform Signal Generator with AM/FM/PM Modulation and Arbitrary Waveform Support
 *
 * 功能: 实现多波形信号发生器，支持正弦/方波/三角/锯齿/噪声/任意波形、
 *       AM/FM/PM调制和线性调频(Chirp)，适用于嵌入式信号测试。
 *
 * 协作: FftEngine(FFT) / BiquadFilter5(滤波器) / MultibandCompressor2(压缩器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多波形信号发生器
 */
class SignalGenerator2 : public QObject {
    Q_OBJECT

public:
    /** @brief 波形类型 */
    enum Waveform { Sine, Square, Triangle, Sawtooth, Noise, Arbitrary };

    /** @brief 调制类型 */
    enum ModType { None, AM, FM, PM };

    /** @brief 调制参数 */
    struct ModParams {
        ModType type = None;
        double modFreq = 10.0;      ///< 调制频率(Hz)
        double modDepth = 0.5;      ///< 调制深度(AM:0~1, FM:Hz偏移, PM:弧度)
        double modPhase = 0.0;      ///< 调制初始相位
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalGenerations = 0;    ///< 累计生成次数
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
        int lastSamples = 0;             ///< 最近采样数
    };

    explicit SignalGenerator2(QObject *parent = nullptr);
    ~SignalGenerator2() override;

    void setSampleRate(double rate);
    void setFrequency(double freq);
    void setAmplitude(double amp);
    void setPhase(double phase);
    void setWaveform(Waveform type);
    void setModulation(const ModParams& mod);
    void setArbitraryWaveform(const QVector<double>& waveform);
    void setDCOffset(double offset);

    /**
     * @brief 生成信号
     * @param numSamples 采样数
     * @return 生成的采样序列
     */
    QVector<double> generate(int numSamples);

    /**
     * @brief 生成线性调频(Chirp)信号
     * @param numSamples 采样数
     * @param startFreq 起始频率(Hz)
     * @param endFreq 结束频率(Hz)
     * @return Chirp采样序列
     */
    QVector<double> generateChirp(int numSamples, double startFreq, double endFreq);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void generationCompleted(int samples);

private:
    /** @brief 基本波形函数 */
    double waveformFunc(double phase) const;

    /** @brief 任意波形插值 */
    double arbitraryInterp(double phase) const;

    double m_sampleRate = 44100.0;
    double m_frequency = 440.0;
    double m_amplitude = 1.0;
    double m_phase = 0.0;
    double m_dcOffset = 0.0;
    Waveform m_waveform = Sine;
    ModParams m_mod;
    QVector<double> m_arbitrary;

    Stats m_stats;
    double m_timeSum = 0.0;
};
