/**
 * @file Deesser4.h
 * @brief 去齿音处理器(MFCC特征提取+深度学习启发式齿音评分+自适应阈值) — De-esser with Deep-Learning-Inspired Sibilance Scoring via MFCC Feature Extraction and Threshold Adaptation
 *
 * 功能: 实现去齿音处理器，支持MFCC特征提取、齿音评分
 *       和自适应增益抑制。
 *
 * 协作: FFTW5(FFT引擎) / SpectralGate4(频谱门) / Equalizer3(均衡器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 去齿音处理器(MFCC特征提取+深度学习启发式齿音评分+自适应阈值)
 */
class Deesser4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int frameSize = 0;
        double avgSibilanceScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser4(QObject *parent = nullptr);
    ~Deesser4() override;

    void setSampleRate(double rate);
    void setFrameSize(int size);
    void setSibilanceThreshold(double thresh);
    void setReductionGain(double gainDb);
    void setAdaptationRate(double rate);

    /** @brief Process a single frame of audio, return de-essed output */
    QVector<double> process(const QVector<double>& frame);

    /** @brief Compute MFCC features for a frame */
    QVector<double> computeMFCC(const QVector<double>& frame) const;

    /** @brief Compute sibilance score from MFCC features (0..1) */
    double sibilanceScore(const QVector<double>& mfcc) const;

    /** @brief Apply frequency-selective gain reduction */
    void applyReduction(QVector<double>& spectrum, double score) const;

    /** @brief Adapt threshold based on running statistics */
    void adaptThreshold(double currentScore);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(double sibilance, double reductionDb, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_frameSize = 2048;
    double m_threshold = 0.6;
    double m_reductionGain = -12.0; // dB
    double m_adaptRate = 0.01;

    // Filter bank for MFCC (26 Mel bands)
    QVector<QVector<double>> m_melFilterBank;
    QVector<double> m_dctCoeffs;
    QVector<double> m_hannWindow;

    // Adaptive state
    double m_runningMean = 0.0;
    double m_runningStd = 0.0;
    double m_sibilanceSum = 0.0;
    int m_frameCount = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Mel-spaced filter bank */
    void buildMelFilterBank();

    /** @brief Build DCT-II coefficients for MFCC */
    void buildDCT();

    /** @brief Build Hann window */
    void buildWindow();

    /** @brief Apply Hann window to frame */
    void applyWindow(QVector<double>& frame) const;

    /** @brief Compute power spectrum from windowed frame */
    QVector<double> powerSpectrum(const QVector<double>& windowed) const;

    /** @brief Convert frequency to Mel scale */
    static double hzToMel(double hz);

    /** @brief Convert Mel to frequency */
    static double melToHz(double mel);
};
