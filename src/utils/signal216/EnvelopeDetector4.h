/**
 * @file EnvelopeDetector4.h
 * @brief 包络检测器(希尔伯特变换解析信号+对数域RMS平滑动态处理) — Envelope Detector with Analytic Signal via Hilbert Transform and Log-Domain RMS Smoothing for Dynamics
 *
 * 功能: 实现包络检测器，支持希尔伯特变换解析信号构造、
 *       对数域RMS平滑和动态范围跟随。
 *
 * 协作: Limiter7(限制器) / Compressor3(压缩器) / FIRFilter2(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器(希尔伯特解析信号+对数域RMS平滑)
 */
class EnvelopeDetector4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        double attackMs = 0.0;
        double releaseMs = 0.0;
        double sampleRate = 0.0;
        double peakEnvelope = 0.0;
        double rmsEnvelope = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetector4(QObject *parent = nullptr);
    ~EnvelopeDetector4() override;

    /** @brief Set attack/release times and sample rate */
    void setParameters(double attackMs = 10.0, double releaseMs = 100.0,
                       double sampleRate = 44100.0);

    /** @brief Process single sample, returns envelope value */
    double processOne(double input);

    /** @brief Process buffer and return envelope */
    QVector<double> process(const QVector<double>& input);

    /** @brief Compute analytic signal via Hilbert transform (FIR approximation) */
    void hilbertTransform(const QVector<double>& input,
                          QVector<double>& analyticReal,
                          QVector<double>& analyticImag) const;

    /** @brief Compute envelope from analytic signal */
    QVector<double> envelopeFromAnalytic(const QVector<double>& real,
                                          const QVector<double>& imag) const;

    /** @brief Apply log-domain RMS smoothing */
    double smoothLogRMS(double currentEnvelope, double newLevel);

    /** @brief Get current envelope level */
    double currentEnvelope() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double peakEnv, double timeMs);

private:
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;
    double m_sampleRate = 44100.0;
    double m_envelope = 0.0;
    double m_logRmsState = 0.0;

    // Hilbert FIR filter coefficients
    QVector<double> m_hilbertCoeffs;
    int m_hilbertDelay = 0;

    // Circular buffer for FIR
    QVector<double> m_history;
    int m_histPos = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design Hilbert FIR filter (odd-length type-III) */
    void designHilbertFilter(int order = 31);
};
