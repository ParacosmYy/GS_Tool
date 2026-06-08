/**
 * @file MultibandGate4.h
 * @brief 多频段门控(Mel尺度子带分解+递归平均噪声轮廓跟踪) — Multiband Gate with Mel-Scale Subband Decomposition and Noise Profile Tracking via Recursive Averaging
 *
 * 功能: 实现多频段噪声门控，使用Mel尺度子带分解，
 *       通过递归平均跟踪噪声轮廓实现自适应门限。
 *
 * 协作: WindowFunction4(窗函数) / Goertzel7(频率检测) / SplitRadixFFT6(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段门控(Mel子带+递归平均噪声轮廓)
 */
class MultibandGate4 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band gate state */
    struct BandState {
        double noiseFloor = 0.0;
        double threshold = 0.0;
        double reduction = 0.0;
        double attackCoeff = 0.0;
        double releaseCoeff = 0.0;
        double gateGain = 1.0;
        int loBin = 0;
        int hiBin = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int fftSize = 0;
        int framesProcessed = 0;
        double avgNoiseFloor = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandGate4(QObject *parent = nullptr);
    ~MultibandGate4() override;

    /** @brief Set parameters: num bands, FFT size, sample rate, thresholds in dB */
    void setParameters(int numBands = 8, int fftSize = 2048, double sampleRate = 44100.0,
                       double thresholdDb = -40.0, double reductionDb = -60.0,
                       double attackMs = 5.0, double releaseMs = 50.0,
                       double noiseAlpha = 0.98);

    /** @brief Learn noise profile from a noise-only frame */
    void learnNoiseProfile(const QVector<double>& noiseFrame);

    /** @brief Process a single frame, returns gated output */
    QVector<double> process(const QVector<double>& inputFrame);

    /** @brief Get current band states */
    QVector<BandState> bandStates() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameNum, double avgNoiseFloor, double timeMs);

private:
    int m_numBands = 8;
    int m_fftSize = 2048;
    double m_sampleRate = 44100.0;
    double m_thresholdDb = -40.0;
    double m_reductionDb = -60.0;
    double m_noiseAlpha = 0.98;

    QVector<BandState> m_bands;
    QVector<double> m_window;
    QVector<double> m_melEdges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Mel-scale frequency band edges */
    void computeMelBands();

    /** @brief Generate Hann analysis window */
    void computeWindow();

    /** @brief Mel frequency conversion */
    double hzToMel(double hz) const;
    double melToHz(double mel) const;

    /** @brief Compute magnitude spectrum via DFT (for sub-band analysis) */
    void computeMagnitude(const QVector<double>& frame, QVector<double>& mag) const;

    /** @brief Recursive average noise estimate update */
    void updateNoiseEstimate(int band, double energy);

    /** @brief Compute gate gain for a band given signal energy */
    double computeGateGain(int band, double energy) const;

    /** @brief Apply gate gains to magnitude spectrum */
    void applyGains(QVector<double>& mag) const;

    /** @brief Reconstruct time-domain signal from modified magnitude */
    QVector<double> reconstruct(const QVector<double>& originalFrame,
                                 const QVector<double>& mag,
                                 const QVector<double>& phase) const;
};
