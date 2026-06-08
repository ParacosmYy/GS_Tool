/**
 * @file MultibandGate4.h
 * @brief 多频段门控(常量Q变换频带分解+独立包络跟随) — Multiband Gate with Constant-Q Transform Band Decomposition and Independent Envelope Followers Per Band
 *
 * 功能: 实现多频段门控(multiband gate)，使用常量Q变换(constant-Q transform, CQT)进行
 *       频带分解(band decomposition)，每个频段配备独立包络跟随器(independent envelope
 *       follower)实现精确的幅度检测与门控控制。
 *
 * 协作: WindowFunction4(窗函数) / Goertzel8(Goertzel算法) / SpectralSubtraction6(谱减法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段门控(常量Q变换频带分解+独立包络跟随)
 */
class MultibandGate4 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band gate state */
    struct BandState {
        double centerFreq = 0.0;
        double bandwidth = 0.0;
        double envelope = 0.0;
        double threshold = 0.0;
        double gain = 0.0;
        bool isOpen = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int numBands = 0;
        int sampleRate = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandGate4(QObject *parent = nullptr);
    ~MultibandGate4() override;

    /** @brief Configure: frame size, sample rate, number of bands */
    bool configure(int frameSize, int sampleRate, int numBands = 8);

    /** @brief Set gate threshold in dB for a specific band */
    void setBandThreshold(int band, double thresholdDb);

    /** @brief Set attack time in ms for envelope follower */
    void setAttack(double ms);

    /** @brief Set release time in ms for envelope follower */
    void setRelease(double ms);

    /** @brief Process one frame of audio samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get current band states */
    QVector<BandState> bandStates() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int frameNum, int activeBands);
    void bandStateChanged(int band, bool isOpen);

private:
    int m_frameSize = 0;
    int m_sampleRate = 0;
    int m_numBands = 0;
    double m_attackCoeff = 0.01;
    double m_releaseCoeff = 0.001;

    QVector<BandState> m_bands;
    QVector<double> m_cqtKernel;    // CQT frequency kernels
    QVector<double> m_window;       // Analysis window

    Stats m_stats;
    double m_timeSum = 0.0;
    int m_frameNum = 0;

    /** @brief Compute CQT band parameters (center freq, bandwidth) */
    void computeCQTBands();

    /** @brief Compute analysis window (Hann) */
    void computeWindow();

    /** @brief Apply CQT decomposition to extract band magnitudes */
    QVector<double> cqtDecompose(const QVector<double>& frame) const;

    /** @brief Apply inverse CQT reconstruction from band gains */
    QVector<double> cqtReconstruct(const QVector<double>& frame,
                                    const QVector<double>& gains) const;

    /** @brief Update envelope follower for a band */
    double updateEnvelope(double current, double sample) const;

    /** @brief Compute gate gain from envelope and threshold */
    double computeGateGain(double envelope, double threshold) const;
};
