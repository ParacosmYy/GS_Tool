/**
 * @file Resampler10.h
 * @brief 重采样器(Farrow结构与Lagrange多项式插值的连续时间分数延迟) — Resampler with Farrow Structure and Lagrange Polynomial Interpolation for Continuous-time Fractional Delay
 *
 * 功能: 实现重采样器(Resampler)，采用Farrow结构(Farrow structure)
 *       与Lagrange多项式插值(Lagrange polynomial interpolation)实现连续时间分数延迟(continuous-time fractional delay)。
 *
 * 协作: Flanger8(镶边) / Chorus7(合唱) / PitchShifter5(变调)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 重采样器(Farrow结构与Lagrange多项式插值)
 */
class Resampler10 : public QObject {
    Q_OBJECT

public:
    /** @brief Farrow filter coefficients for Lagrange interpolation */
    struct FarrowCoeffs {
        int order = 3;               // Polynomial order (3 = cubic)
        QVector<QVector<double>> c;  // c[order+1][order+1] coefficient matrix
    };

    /** @brief Resampling result */
    struct ResampleResult {
        QVector<double> output;
        int inputSize = 0;
        int outputSize = 0;
        double ratio = 1.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputSamples = 0;
        int outputSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Resampler10(QObject *parent = nullptr);
    ~Resampler10() override;

    /** @brief Set Farrow filter order */
    void setFilterOrder(int order);

    /** @brief Resample by arbitrary ratio */
    ResampleResult resample(const QVector<double>& input, double ratio);

    /** @brief Apply fractional delay to signal */
    QVector<double> fractionalDelay(const QVector<double>& input, double delay) const;

    /** @brief Compute Farrow output for single fractional position */
    double farrowOutput(const QVector<double>& samples, double mu) const;

    /** @brief Get current Farrow coefficients */
    const FarrowCoeffs& farrowCoeffs() const { return m_farrow; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resampleDone(int inSize, int outSize, double ratio, double timeMs);

private:
    FarrowCoeffs m_farrow;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Lagrange-Farrow coefficients for given order */
    void computeFarrowCoeffs();

    /** @brief Lagrange basis polynomial value at mu */
    double lagrangeBasis(int k, int n, double mu) const;

    /** @brief Get samples centered around position (with boundary handling) */
    QVector<double> getSamples(const QVector<double>& input,
                                int centerIdx, int halfLen) const;
};
