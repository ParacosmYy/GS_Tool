/**
 * @file Delay5.h
 * @brief 磁带延迟仿真(抖晃/颤动建模+调制全通+磁饱和非线性) — Tape Delay Emulation with Wow/Flutter Modeling via Modulated Allpass and Magnetic Saturation Nonlinearity
 *
 * 功能: 实现磁带延迟效果器仿真，包含磁带抖晃(wow/flutter)的调制全通滤波器建模、
 *       磁饱和非线性失真、以及磁带损耗低通滤波器。
 *
 * 协作: AllPassFilter2(全通滤波器) / BiquadFilter1(双二阶) / WaveShaper3(波形塑形)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 磁带延迟仿真(抖晃颤动+磁饱和)
 */
class Delay5 : public QObject {
    Q_OBJECT

public:
    /** @brief Tape delay parameters */
    struct Parameters {
        double delayTimeMs = 200.0;    // base delay in ms
        double feedback = 0.45;        // feedback gain [0..1)
        double wowRate = 0.8;          // wow LFO frequency (Hz)
        double wowDepth = 0.002;       // wow modulation depth
        double flutterRate = 6.0;      // flutter LFO frequency (Hz)
        double flutterDepth = 0.0008;  // flutter modulation depth
        double saturation = 0.5;       // tape saturation amount
        double tapeLossHz = 4000.0;    // tape loss lowpass cutoff
        double mix = 0.5;              // dry/wet mix
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int sampleRate = 44100;
        int bufferSize = 0;
        int numProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Delay5(QObject *parent = nullptr);
    ~Delay5() override;

    /** @brief Initialize delay line with sample rate */
    bool init(int sampleRate, double maxDelayMs = 2000.0);

    /** @brief Set tape delay parameters */
    void setParameters(const Parameters& params);

    /** @brief Process single sample */
    double processSample(double input);

    /** @brief Process buffer in-place */
    void processBuffer(QVector<double>& buffer);

    /** @brief Reset delay line and LFO phase */
    void reset();

    const Parameters& parameters() const { return m_params; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    Parameters m_params;
    Stats m_stats;
    double m_timeSum = 0.0;

    int m_sampleRate = 44100;
    double m_maxDelayMs = 2000.0;

    // Circular delay buffer
    QVector<double> m_delayBuffer;
    int m_writePos = 0;
    int m_bufferSize = 0;

    // LFO state
    double m_wowPhase = 0.0;
    double m_flutterPhase = 0.0;

    // Allpass modulator state
    double m_allpassX1 = 0.0;
    double m_allpassY1 = 0.0;

    // Tape loss lowpass state (1st-order IIR)
    double m_lossZ1 = 0.0;

    /** @brief Compute modulated delay time with wow/flutter */
    double modulatedDelay() const;

    /** @brief Apply magnetic saturation soft clipping */
    double saturate(double x) const;

    /** @brief Tape loss lowpass filter (1st-order) */
    double tapeLossFilter(double input);

    /** @brief Modulated allpass for wow/flutter coloration */
    double allpassModulate(double input);

    /** @brief Fractional delay read with linear interpolation */
    double readDelay(double delaySamples) const;
};
