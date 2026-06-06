/**
 * @file EnvelopeDetector3.h
 * @brief 包络检测(Hilbert变换+解析信号+瞬时频率) — Envelope Detection via Hilbert Transform with Analytic Signal and Instantaneous Frequency
 *
 * 功能: 实现信号包络检测，支持Hilbert变换生成解析信号、
 *       包络提取、瞬时相位和瞬时频率计算。
 *
 * 协作: FIRFilter4(FIR滤波器) / Goertzel5(Goertzel) / WaveletTransform8(小波变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器(Hilbert变换+解析信号)
 */
class EnvelopeDetector3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDetections = 0;
        int signalLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EnvelopeDetector3(QObject *parent = nullptr);
    ~EnvelopeDetector3() override;

    /** @brief 计算Hilbert变换 */
    QVector<double> hilbertTransform(const QVector<double>& signal) const;

    /** @brief 计算解析信号(实部=原信号,虚部=Hilbert) */
    void analyticSignal(const QVector<double>& signal,
                        QVector<double>& realPart,
                        QVector<double>& imagPart) const;

    /** @brief 提取包络(解析信号幅度) */
    QVector<double> envelope(const QVector<double>& signal);

    /** @brief 计算瞬时相位 */
    QVector<double> instantaneousPhase(const QVector<double>& signal) const;

    /** @brief 计算瞬时频率 */
    QVector<double> instantaneousFrequency(const QVector<double>& signal,
                                            double sampleRate = 1.0) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(int length, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Next power of 2 */
    int nextPow2(int n) const;

    /** @brief Radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im, bool inverse) const;
};
