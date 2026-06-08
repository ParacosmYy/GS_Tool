/**
 * @file EnvelopeDetector5.h
 * @brief 包络检测(峰值保持衰减建模+RMS峰值比动态分类) — Envelope Detector with Peak-hold Decay Modeling and RMS-to-peak Ratio Estimation for Dynamics Classification
 *
 * 功能: 实现包络检测器，使用峰值保持衰减建模(peak-hold decay modeling)跟踪信号包络，
 *       通过RMS-峰值比(RMS-to-peak ratio)估计进行动态特性分类。
 *
 * 协作: Limiter8(限幅器) / Compressor4(压缩器) / PeakMeter2(峰值表)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器(峰值保持衰减+RMS峰值比动态分类)
 */
class EnvelopeDetector5 : public QObject {
    Q_OBJECT

public:
    /** @brief Dynamics classification result */
    enum class DynamicsClass {
        Flat = 0,       // RMS/peak ~ 1.0 (square wave)
        Compressed = 1, // RMS/peak ~ 0.7-0.9
        Normal = 2,     // RMS/peak ~ 0.5-0.7
        Dynamic = 3,    // RMS/peak ~ 0.3-0.5
        Sparse = 4      // RMS/peak < 0.3 (impulsive)
    };

    /** @brief Detection result for a block */
    struct BlockResult {
        double peakEnvelope = 0.0;
        double rmsEnvelope = 0.0;
        double rmsToPeakRatio = 0.0;
        DynamicsClass dynamicsClass = DynamicsClass::Normal;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numBlocks = 0;
        double avgRmsToPeak = 0.0;
        double peakHoldMax = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetector5(int sampleRate = 44100,
                                 QObject *parent = nullptr);
    ~EnvelopeDetector5() override;

    /** @brief Set attack/release times in ms and peak-hold time */
    void setParameters(double attackMs, double releaseMs,
                        double peakHoldMs);

    /** @brief Process audio block and return envelope */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process single sample, return envelope value */
    double processSample(double sample);

    /** @brief Analyze a block for dynamics classification */
    BlockResult analyzeBlock(const QVector<double>& block) const;

    /** @brief Reset detector state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int samples, double rmsToPeak, double timeMs);
    void dynamicsClassified(int blockNum, int dynamicsClass);

private:
    int m_sampleRate;
    double m_attackCoeff = 0.0;
    double m_releaseCoeff = 0.0;
    int m_peakHoldSamples = 0;

    double m_peakEnvelope = 0.0;
    double m_rmsEnvelope = 0.0;
    double m_peakHoldValue = 0.0;
    int m_peakHoldCounter = 0;

    double m_rmsAccum = 0.0;
    int m_rmsCount = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_rmsToPeakSum = 0.0;

    /** @brief Classify dynamics from RMS-to-peak ratio */
    DynamicsClass classify(double rmsToPeak) const;
};
