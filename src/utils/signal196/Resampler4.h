/**
 * @file Resampler4.h
 * @brief 多相位重采样器(CIC抗混叠+有理数速率转换) — Polyphase Resampler with Cascaded Integrator-Comb Anti-Aliasing and Rational Rate Conversion
 *
 * 功能: 实现多相位重采样器，集成CIC抗混叠滤波、
 *       有理数速率转换(P/Q)和多相位分支滤波。
 *
 * 协作: FIRFilter3(FIR滤波) / Decimator4(抽取) / Interpolator3(插值)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 多相位重采样器(CIC+有理数速率转换)
 */
class Resampler4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;
        int inputRate = 0;
        int outputRate = 0;
        double cicOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Resampler4(QObject *parent = nullptr);
    ~Resampler4() override;

    /** @brief Set resampling ratio P/Q (output = input * P/Q) */
    void setRatio(int P, int Q);

    /** @brief Set CIC filter order (number of stages) */
    void setCicOrder(int order);

    /** @brief Set polyphase filter taps per branch */
    void setTapsPerBranch(int taps);

    /** @brief Process input samples, return resampled output */
    QVector<double> process(const QVector<double>& input);

    /** @brief Compute CIC impulse response */
    QVector<double> cicResponse(int length) const;

    /** @brief Get polyphase filter coefficients for branch b */
    QVector<double> polyphaseBranch(int b) const;

    /** @brief Get current P/Q ratio */
    QPair<int, int> ratio() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int inSamples, int outSamples, double timeMs);

private:
    int m_P = 1;        // interpolation factor
    int m_Q = 1;        // decimation factor
    int m_cicOrder = 4;
    int m_tapsPerBranch = 8;

    // CIC filter state (integrator + comb)
    QVector<double> m_intState;   // integrator accumulators
    QVector<double> m_combState;  // comb delay lines

    // Polyphase filter bank
    QVector<QVector<double>> m_branches;

    // Fractional sample position
    double m_phase = 0.0;

    // Input buffer for overlapping
    QVector<double> m_buffer;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design polyphase filter from CIC-compensated prototype */
    void designFilters();

    /** @brief Apply CIC integrator stage */
    double cicIntegrate(double sample);

    /** @brief Apply CIC comb stage */
    double cicComb(double sample);

    /** @brief GCD */
    static int gcd(int a, int b);
};
