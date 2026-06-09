/**
 * @file Resampler8.h
 * @brief 重采样器(窗函数sinc插值+多相滤波器组) — Resampler with Windowed Sinc Interpolation and Polyphase Filter Bank for High-Quality Asynchronous Sample Rate Conversion
 *
 * 功能: 实现高质量异步采样率转换(asynchronous sample rate conversion)，
 *       使用窗函数sinc插值(windowed sinc interpolation)计算理想低通滤波器
 *       冲激响应，多相滤波器组(polyphase filter bank)实现高效实时处理。
 *
 * 协作: Flanger6(镶边效果) / Flanger6(镶边) / Window7(窗函数) / Filter6(滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 重采样器(窗函数sinc插值+多相滤波器组)
 */
class Resampler8 : public QObject {
    Q_OBJECT

public:
    /** @brief Window type for sinc filter design */
    enum Window { Blackman, Hann, Lanczos, Kaiser };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputRate = 0;
        int outputRate = 0;
        int inputSamples = 0;
        int outputSamples = 0;
        int filterLength = 0;
        int numPhases = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Resampler8(QObject *parent = nullptr);
    ~Resampler8() override;

    /** @brief Configure resampler: input rate, output rate, filter taps */
    bool prepare(int inputRate, int outputRate, int filterTaps = 64);

    /** @brief Set window type for sinc filter design */
    void setWindow(Window win);

    /** @brief Process input samples, return resampled output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Flush remaining samples from filter state */
    QVector<double> flush();

    /** @brief Get current phase position (0..1) */
    double phase() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processCompleted(int inSamples, int outSamples, double timeMs);

private:
    int m_inputRate = 44100;
    int m_outputRate = 48000;
    int m_filterTaps = 64;
    Window m_window = Blackman;

    double m_ratio = 1.0;      // outputRate / inputRate
    double m_phase = 0.0;      // Current fractional phase
    double m_phaseStep = 0.0;  // Phase increment per output sample

    // Polyphase filter bank: m_polyPhase[phase][tap]
    QVector<QVector<double>> m_polyPhase;
    int m_numPhases = 32;

    // Filter state (delay line)
    QVector<double> m_delayLine;
    int m_delayPos = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design windowed sinc lowpass filter */
    QVector<double> designSincFilter() const;

    /** @brief Decompose filter into polyphase branches */
    void buildPolyPhase(const QVector<double>& filter);

    /** @brief Apply window function */
    double applyWindow(int n, int N) const;

    /** @brief Process one output sample from delay line */
    double interpolate(int phaseIndex) const;
};
