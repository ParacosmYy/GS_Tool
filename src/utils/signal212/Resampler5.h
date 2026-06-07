/**
 * @file Resampler5.h
 * @brief 采样率转换器(多相FIR插值+级联积分梳状滤波器抽取) — Sample Rate Converter with Polyphase FIR Interpolation and Cascaded Integrator-Comb Decimation Stage
 *
 * 功能: 实现采样率转换器，支持多相FIR插值、
 *       级联积分梳状(CIC)抽取和任意比采样率转换。
 *
 * 协作: Flanger3(镶边) / FIRFilter6(FIR滤波器) / WindowFunction5(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 采样率转换器(多相FIR插值+级联积分梳状滤波器抽取)
 */
class Resampler5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputRate = 0;
        int outputRate = 0;
        int cicOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief CIC decimation stage parameters */
    struct CICParams {
        int decimationFactor = 1;
        int order = 3;              // Number of integrator-comb pairs
        int differentialDelay = 1;  // Usually 1 or 2
    };

    explicit Resampler5(QObject *parent = nullptr);
    ~Resampler5() override;

    void setInputRate(int rate);
    void setOutputRate(int rate);
    void setFIRLength(int length);

    /** @brief Initialize polyphase FIR interpolator and CIC decimator */
    void initialize();

    /** @brief Resample a block using polyphase FIR + CIC pipeline */
    QVector<double> process(const QVector<double>& input);

    /** @brief Polyphase FIR interpolation by factor L */
    QVector<double> interpolatePolyphase(
        const QVector<double>& input, int L) const;

    /** @brief CIC decimation by factor M */
    QVector<double> decimateCIC(
        const QVector<double>& input, int M);

    /** @brief Design a lowpass FIR filter for interpolation */
    QVector<double> designLowpassFIR(int length, double cutoff) const;

    /** @brief Get polyphase subfilter bank */
    QVector<QVector<double>> polyphaseDecompose(
        const QVector<double>& firCoeffs, int L) const;

    /** @brief Reset CIC integrator/comb state */
    void resetCIC();

    /** @brief Compute the rational resampling ratio (L/M) */
    void computeRatio(int& L, int& M) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resamplingCompleted(int inputSize, int outputSize, double timeMs);

private:
    int m_inputRate = 44100;
    int m_outputRate = 48000;
    int m_firLength = 64;

    QVector<QVector<double>> m_polyFilters;  // Polyphase subfilters
    CICParams m_cicParams;
    QVector<double> m_integratorState;       // CIC integrator registers
    QVector<double> m_combState;             // CIC comb delay lines
    int m_cicPhase = 0;                      // CIC decimation phase counter
    bool m_initialized = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sinc function for FIR design */
    static double sinc(double x);

    /** @brief Blackman window */
    static double blackman(int n, int N);

    /** @brief GCD for ratio computation */
    static int gcd(int a, int b);
};
