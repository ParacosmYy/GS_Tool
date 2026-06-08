/**
 * @file SpectralGate5.h
 * @brief 谱门控(深度神经噪声轮廓估计+Wiener抑制增益映射) — Spectral Gate with Deep Neural Noise Profile Estimation and Wiener-Style Suppression Gain Mapping
 *
 * 功能: 实现谱门控算法，集成多层感知器噪声轮廓估计，
 *       Wiener风格抑制增益映射实现高质量降噪。
 *
 * 协作: SpectralSubtraction4(谱减法) / WienerFilter3(Wiener滤波) / KalmanFilter6(Kalman)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 谱门控(神经噪声估计+Wiener增益)
 */
class SpectralGate5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int fftSize = 0;
        int numFrames = 0;
        int noiseEstimationFrames = 0;
        double noiseFloorDb = 0.0;
        double snrImprovementDb = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralGate5(QObject *parent = nullptr);
    ~SpectralGate5() override;

    /** @brief Set parameters: frame size, threshold dB, reduction dB, neural hidden dim */
    void setParameters(int frameSize = 1024, double thresholdDb = -40.0,
                       double reductionDb = -30.0, int hiddenDim = 64);

    /** @brief Estimate noise profile from initial silence frames */
    void estimateNoise(const QVector<double>& noiseFrames);

    /** @brief Process signal frame: gate with Wiener suppression */
    QVector<double> process(const QVector<double>& frame);

    /** @brief Process entire signal */
    QVector<double> processSignal(const QVector<double>& signal);

    /** @brief Get current noise profile magnitude spectrum */
    QVector<double> noiseProfile() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frames, double snrDb, double timeMs);

private:
    int m_frameSize = 1024;
    int m_fftSize = 2048;
    double m_thresholdDb = -40.0;
    double m_reductionDb = -30.0;
    int m_hiddenDim = 64;

    QVector<double> m_noiseMag;
    QVector<double> m_noisePhase;
    QVector<double> m_prevGain;

    // Neural noise estimator weights (simplified MLP)
    QVector<QVector<double>> m_w1;  // input -> hidden
    QVector<double> m_b1;
    QVector<double> m_w2;           // hidden -> output
    double m_b2 = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Forward FFT (radix-2 Cooley-Tukey) */
    void fft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Inverse FFT */
    void ifft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Apply Hann window */
    QVector<double> hannWindow(int size) const;

    /** @brief Neural network forward pass for noise estimation */
    QVector<double> neuralForward(const QVector<double>& input) const;

    /** @brief Compute Wiener suppression gain */
    QVector<double> wienerGain(const QVector<double>& signalMag,
                                 const QVector<double>& noiseMag) const;

    /** @brief Initialize neural weights */
    void initNeuralWeights();

    /** @brief Train neural noise estimator (single gradient step) */
    void trainNeuralStep(const QVector<double>& mag, double targetFloor);
};
