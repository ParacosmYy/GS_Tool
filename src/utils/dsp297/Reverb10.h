/**
 * @file Reverb10.h
 * @brief 混响(后期反射FDN反馈延迟网络与早期反射随机光线追踪实现自然房间混响模拟) — Reverb with Late-reflection FDN Feedback Delay Network and Early-reflection Stochastic Ray Tracing for Natural Room Simulation
 *
 * 功能: 实现混响(reverb)，采用后期反射FDN反馈延迟网络(late-reflection FDN feedback delay network)
 *       与早期反射随机光线追踪(early-reflection stochastic ray tracing)实现自然房间混响模拟(natural room simulation)。
 *
 * 协作: SignalGenerator10(信号发生器) / FIRFilter(FIR滤波器) / IIRFilter(IIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

class Reverb10 : public QObject {
    Q_OBJECT

public:
    /** @brief Room parameters for simulation */
    struct RoomParams {
        double roomSize = 1.0;       // 0..1 normalized
        double damping = 0.5;        // high-freq absorption
        double wetLevel = 0.3;
        double dryLevel = 0.7;
        double earlyGain = 0.8;
        double lateGain = 0.6;
        double diffusion = 0.7;
    };

    /** @brief Processing result */
    struct ProcessResult {
        QVector<double> output;
        double earlyEnergy = 0.0;
        double lateEnergy = 0.0;
        double totalTimeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalProcessings = 0;
        int lastFrameCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Reverb10(QObject *parent = nullptr);
    ~Reverb10() override;

    void setRoomParams(const RoomParams& params);
    void setSampleRate(double sr);

    /** @brief Process audio through reverb */
    ProcessResult process(const QVector<double>& input);

    /** @brief Reset delay line state */
    void flush();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processDone(int frames, double timeMs);

private:
    RoomParams m_params;
    double m_sampleRate = 44100.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief FDN delay lines (8 lines for late reverb) */
    static const int kFDNOrder = 8;
    QVector<QVector<double>> m_delayLines;
    QVector<int> m_delayLengths;
    QVector<int> m_writePos;

    /** @brief Early reflection delay taps and gains */
    QVector<int> m_earlyTaps;
    QVector<double> m_earlyGains;

    /** @brief FDN feedback matrix (Hadamard-based) */
    double m_feedbackMatrix[kFDNOrder][kFDNOrder];

    /** @brief Initialize FDN delay lengths (Schroeder series) */
    void initFDN();

    /** @brief Initialize early reflection taps via stochastic ray tracing */
    void initEarlyReflections();

    /** @brief Process early reflections */
    QVector<double> processEarly(const QVector<double>& input);

    /** @brief Process late reverb via FDN */
    QVector<double> processLate(const QVector<double>& input);
};
