/**
 * @file Deesser11.h
 * @brief 去齿音器(性别自适应齿音带检测与动态阈值跟踪实现跨语音类型透明去齿音) — De-esser with Gender-Adaptive Sibilance Band Detection and Dynamic Threshold Tracking for Transparent Vocal De-essing Across Voice Types
 *
 * 功能: 实现去齿音器(de-esser)，采用性别自适应齿音带检测(gender-adaptive sibilance band detection)
 *       与动态阈值跟踪(dynamic threshold tracking)实现跨语音类型透明去齿音(transparent vocal de-essing across voice types)。
 *
 * 协作: FFTAnalyzer(FFT分析) / Compressor(压缩器) / Equalizer(均衡器)
 */
#pragma once

#include <QObject>
#include <QVector>

class Deesser11 : public QObject {
    Q_OBJECT

public:
    /** @brief Voice gender profile for adaptive band selection */
    enum class VoiceProfile { Male, Female, AutoDetect };

    /** @brief Processing result for a frame */
    struct FrameResult {
        QVector<double> output;
        double sibilanceLevel = 0.0;
        double gainReduction = 0.0;
        bool sibilanceDetected = false;
    };

    /** @brief Processing statistics */
    struct Stats {
        quint64 totalFrames = 0;
        int sampleRate = 44100;
        double avgGainReduction = 0.0;
        double sibilanceRate = 0.0;       // Ratio of frames with detected sibilance
        double avgProcessingTimeMs = 0.0;
    };

    explicit Deesser11(QObject *parent = nullptr);
    ~Deesser11() override;

    void setSampleRate(int sr);
    void setThreshold(double dB);
    void setRatio(double ratio);
    void setAttack(double ms);
    void setRelease(double ms);
    void setVoiceProfile(VoiceProfile profile);

    /** @brief Process a single frame of audio samples */
    FrameResult processFrame(const QVector<double>& input);

    /** @brief Process entire audio buffer */
    QVector<double> process(const QVector<double>& input);

    /** @brief Detect voice profile from audio */
    VoiceProfile detectVoiceProfile(const QVector<double>& audio) const;

    /** @brief Get current sibilance band frequency range */
    QPair<double, double> sibilanceBand() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(double sibilanceLevel, double gainReduction, bool detected);
    void profileDetected(VoiceProfile profile);

private:
    int m_sampleRate = 44100;
    double m_thresholdDb = -20.0;
    double m_ratio = 4.0;
    double m_attackMs = 0.5;
    double m_releaseMs = 50.0;
    VoiceProfile m_profile = VoiceProfile::AutoDetect;

    // Band-pass filter state for sibilance detection
    QVector<double> m_bpX1, m_bpX2, m_bpY1, m_bpY2;
    double m_envFollower = 0.0;          // Envelope follower state
    double m_dynamicThreshold = -20.0;   // Adaptive threshold
    double m_gainReduction = 1.0;        // Current gain (0..1)
    int m_sibilanceCount = 0;
    int m_frameCount = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design band-pass filter coefficients for sibilance band */
    void designBandpass(double lowFreq, double highFreq);

    /** @brief Apply band-pass filter to extract sibilance energy */
    QVector<double> bandpassFilter(const QVector<double>& input);

    /** @brief Compute envelope of filtered signal */
    double computeEnvelope(const QVector<double>& filtered);

    /** @brief Convert dB to linear */
    double dbToLinear(double dB) const;

    /** @brief Convert linear to dB */
    double linearToDb(double linear) const;

    /** @brief Get sibilance frequency range based on voice profile */
    QPair<double, double> getBandForProfile(VoiceProfile p) const;
};
