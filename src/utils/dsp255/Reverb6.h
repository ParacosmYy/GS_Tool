/**
 * @file Reverb6.h
 * @brief 混响器(反馈延迟网络+哈达玛矩阵混合扩散空间场) — Reverb with Feedback Delay Network and Hadamard Matrix Mixing for Diffuse Spatial Field Simulation
 *
 * 功能: 实现反馈延迟网络混响器(Feedback Delay Network reverb)，使用
 *       哈达玛矩阵混合(Hadamard matrix mixing)实现无损信号散射，
 *       模拟扩散空间场(diffuse spatial field simulation)。
 *
 * 协作: Reverb5(Schroeder混响) / FDL5(分数延迟) / IIRFilter7(IIR滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 混响器(FDN哈达玛矩阵扩散空间场)
 */
class Reverb6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFramesProcessed = 0;
        int fdnOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Reverb6(QObject *parent = nullptr);
    ~Reverb6() override;

    /** @brief Set sample rate */
    void setSampleRate(double sr);

    /** @brief Set room size (0.0..1.0) */
    void setRoomSize(double size);

    /** @brief Set decay time in seconds (T60) */
    void setDecayTime(double t60);

    /** @brief Set wet/dry mix (0.0=dry, 1.0=wet) */
    void setWetDry(double mix);

    /** @brief Initialize FDN with given order (must be power of 2) */
    bool init(int fdnOrder = 8);

    /** @brief Process a block of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Reset delay lines to zero */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frames, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_roomSize = 0.5;
    double m_t60 = 2.0;
    double m_wetDry = 0.3;
    int m_order = 0;

    QVector<QVector<double>> m_delayLines;   // FDN delay line buffers
    QVector<int> m_delayLengths;              // Delay length per line
    QVector<int> m_writePos;                  // Write cursor per line
    QVector<double> m_feedbackGains;          // Per-line feedback gain
    QVector<QVector<double>> m_hadamard;      // Hadamard mixing matrix

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Hadamard matrix of given order */
    void buildHadamard(int n);

    /** @brief Calculate delay lengths from room size and order */
    void computeDelayLengths();

    /** @brief Calculate feedback gains from T60 and delay lengths */
    void computeFeedbackGains();

    /** @brief Apply tone damping (low-pass per delay line) */
    double damp(int lineIdx, double sample) const;

    static bool isPowerOf2(int n);
};
