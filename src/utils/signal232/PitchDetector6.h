/**
 * @file PitchDetector6.h
 * @brief 音高检测(倒频谱基频估计+多候选HMM音高跟踪+浊音/清音判决) — Pitch Detector with Cepstrum-based Fundamental Frequency and Multi-candidate HMM Pitch Tracking with Voicing Decision
 *
 * 功能: 实现音高检测，使用倒频谱(cepstrum)估计基频(fundamental frequency)，
 *       结合多候选隐马尔可夫模型(HMM)进行音高轨迹跟踪(pitch tracking)，并做浊音/清音判决(voicing decision)。
 *
 * 协作: FFTCore5(FFT核心) / WindowFunc3(窗函数) / SignalFilter2(信号滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 音高检测(倒频谱基频+多候选HMM音高跟踪+浊清音判决)
 */
class PitchDetector6 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-frame pitch result */
    struct PitchFrame {
        double frequency = 0.0;     // Hz, 0 if unvoiced
        double confidence = 0.0;    // [0, 1]
        bool voiced = false;
        double energy = 0.0;
    };

    /** @brief HMM candidate for pitch tracking */
    struct HMMCandidate {
        double frequency = 0.0;
        double probability = 0.0;
        int prevCandidate = -1;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFrames = 0;
        int frameSize = 0;
        int hopSize = 0;
        double avgF0 = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PitchDetector6(QObject *parent = nullptr);
    ~PitchDetector6() override;

    /** @brief Configure: frame size, hop size, sample rate, F0 range */
    bool configure(int frameSize, int hopSize, double sampleRate,
                   double f0Min = 50.0, double f0Max = 600.0);

    /** @brief Detect pitch for a single frame */
    PitchFrame detectFrame(const QVector<double>& frame);

    /** @brief Detect pitch trajectory for full signal with HMM tracking */
    QVector<PitchFrame> detectTrajectory(const QVector<double>& signal);

    /** @brief Get cepstrum of last frame */
    QVector<double> cepstrum() const;

    /** @brief Get pitch candidates from last frame */
    QVector<HMMCandidate> candidates() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameDetected(double f0, bool voiced, double confidence, double timeMs);
    void trajectoryCompleted(int numFrames, double avgF0, double timeMs);

private:
    int m_frameSize = 2048;
    int m_hopSize = 512;
    double m_sampleRate = 44100.0;
    double m_f0Min = 50.0;
    double m_f0Max = 600.0;
    int m_numCandidates = 5;

    QVector<double> m_lastCepstrum;
    QVector<HMMCandidate> m_lastCandidates;

    // HMM tracking state
    QVector<QVector<HMMCandidate>> m_trellis;
    double m_transitionStd = 0.5;   // semitone std for transition

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute real cepstrum via FFT */
    QVector<double> computeCepstrum(const QVector<double>& frame) const;

    /** @brief Find peak in cepstrum within quefrency range corresponding to F0 */
    double findCepstralPeak(const QVector<double>& cep, double& confidence) const;

    /** @brief Generate N best pitch candidates from cepstrum */
    QVector<HMMCandidate> generateCandidates(const QVector<double>& cep) const;

    /** @brief Compute voicing probability based on energy and peak prominence */
    double voicingProbability(double energy, double peakProminence) const;

    /** @brief HMM transition cost between two frequencies (in semitones) */
    double transitionCost(double f1, double f2) const;

    /** @brief Viterbi backtrace to extract best pitch path */
    QVector<PitchFrame> viterbiBacktrace(
        const QVector<QVector<HMMCandidate>>& trellis) const;

    /** @brief Frame energy (RMS) */
    double frameEnergy(const QVector<double>& frame) const;
};
