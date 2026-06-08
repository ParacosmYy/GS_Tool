/**
 * @file NoiseGate5.h
 * @brief 噪声门(谱减法门控+自适应攻击释放瞬态检测) — Noise Gate with Spectral Subtraction Gating and Adaptive Attack/Release Based on Transient Detection
 *
 * 功能: 实现基于谱减法的噪声门，通过瞬态检测自适应调整
 *       攻击和释放时间，有效抑制背景噪声同时保留信号瞬态。
 *
 * 协作: SpectralSubtraction4(谱减法) / AdaptiveFilter5(自适应滤波) / Compressor3(压缩器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门(谱减法门控+自适应攻击释放)
 */
class NoiseGate5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int sampleRate = 0;
        int framesProcessed = 0;
        double gateOpenRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NoiseGate5(QObject *parent = nullptr);
    ~NoiseGate5() override;

    /** @brief Set parameters: threshold dB, attack ms, release ms, frame size, sample rate */
    void setParameters(double thresholdDb = -40.0,
                       double attackMs = 1.0,
                       double releaseMs = 50.0,
                       int frameSize = 1024,
                       int sampleRate = 44100);

    /** @brief Estimate noise profile from noise-only frames */
    void estimateNoise(const QVector<double>& noiseSamples);

    /** @brief Process a single frame through the noise gate */
    QVector<double> process(const QVector<double>& frame);

    /** @brief Process entire signal */
    QVector<double> processSignal(const QVector<double>& signal);

    /** @brief Detect transient onset in frame */
    bool detectTransient(const QVector<double>& frame) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameIndex, double levelDb, bool gateOpen);
    void processingCompleted(int totalFrames, double timeMs);

private:
    double m_thresholdDb = -40.0;
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;
    int m_frameSize = 1024;
    int m_sampleRate = 44100;

    double m_gain = 0.0;         // Current gain state (0=closed, 1=open)
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;
    double m_prevEnergy = 0.0;

    QVector<double> m_noiseProfile;  // Estimated noise spectrum magnitude
    bool m_noiseEstimated = false;

    Stats m_stats;
    double m_timeSum = 0.0;
    int m_gateOpenCount = 0;

    /** @brief Compute frame RMS energy in dB */
    double frameEnergyDb(const QVector<double>& frame) const;

    /** @brief Spectral subtraction on frequency domain */
    QVector<double> spectralSubtract(const QVector<double>& frame) const;

    /** @brief Update attack/release coefficients */
    void updateCoefficients();

    /** @brief Apply gain envelope to frame */
    QVector<double> applyGain(const QVector<double>& frame, double targetGain);
};
