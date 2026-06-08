/**
 * @file Reverb3.h
 * @brief 算法混响(Schroeder全通级联+Hadamard混合反馈延迟网络) — Algorithmic Reverb with Schroeder Allpass Cascade and Feedback Delay Network with Hadamard Mixing
 *
 * 功能: 实现算法混响，支持Schroeder全通级联、
 *       Hadamard混合矩阵FDN和参数化房间仿真。
 *
 * 协作: SignalGenerator4(信号发生器) / BiquadFilter3(双二阶滤波器) / FIRFilter2(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 算法混响(Schroeder全通+Hadamard FDN)
 */
class Reverb3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalProcessed = 0;
        int delayLines = 0;
        double roomSize = 0.0;
        double sampleRate = 44100.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Reverb3(QObject *parent = nullptr);
    ~Reverb3() override;

    /** @brief Set reverb parameters */
    void setParameters(double roomSize, double damping, double wetLevel,
                       double sampleRate = 44100.0);

    /** @brief Process a single sample */
    double processOne(double input);

    /** @brief Process a buffer of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Reset all delay lines */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    static const int kFDNLines = 4;
    static const int kAllpassCount = 4;

    double m_roomSize = 0.5;
    double m_damping = 0.5;
    double m_wet = 0.3;
    double m_dry = 0.7;
    double m_sampleRate = 44100.0;
    double m_feedback = 0.84;

    // Schroeder allpass cascade
    QVector<QVector<double>> m_apBuf;
    QVector<int> m_apPos;
    QVector<double> m_apGain;

    // FDN delay lines
    QVector<QVector<double>> m_fdnBuf;
    QVector<int> m_fdnPos;
    QVector<int> m_fdnLen;

    // Damping low-pass state
    QVector<double> m_lpState;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize delay line lengths from room size */
    void initDelayLines();

    /** @brief 4x4 Hadamard mixing matrix */
    static void hadamardMix(double v[4]);

    /** @brief Process one sample through allpass cascade */
    double processAllpass(double input);

    /** @brief Process one sample through FDN */
    double processFDN(double input);
};
