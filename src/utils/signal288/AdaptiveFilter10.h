/**
 * @file AdaptiveFilter10.h
 * @brief 自适应滤波器(递归最小二乘与指数遗忘因子快速收敛系统辨识) — Adaptive Filter with Recursive Least Squares and Exponential Forgetting Factor for Fast-converging System Identification
 *
 * 功能: 实现自适应滤波器(Adaptive filter)，采用递归最小二乘(RLS)
 *       与指数遗忘因子(exponential forgetting factor)实现快速收敛系统辨识(fast-converging system identification)。
 *
 * 协作: BiquadFilter9(双二阶滤波) / KalmanFilter8(卡尔曼滤波) / MultibandCompressor10(多频段压缩)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 自适应滤波器(递归最小二乘与指数遗忘因子快速收敛系统辨识)
 */
class AdaptiveFilter10 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter configuration */
    struct FilterConfig {
        int filterOrder = 32;           // Number of taps
        double forgettingFactor = 0.99; // Lambda (0.9 - 1.0)
        double regularization = 1e-6;   // Delta for P(0) initialization
    };

    /** @brief Filter output frame */
    struct FilterOutput {
        double output = 0.0;
        double error = 0.0;
        double weightsNorm = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int filterOrder = 0;
        double avgError = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AdaptiveFilter10(QObject *parent = nullptr);
    ~AdaptiveFilter10() override;

    void setConfig(const FilterConfig& cfg);

    /** @brief Process single sample pair (input, desired) */
    FilterOutput processSample(double input, double desired);

    /** @brief Process block of samples */
    QVector<FilterOutput> processBlock(const QVector<double>& input,
                                        const QVector<double>& desired);

    /** @brief Get current filter weights */
    QVector<double> weights() const { return m_w; }

    /** @brief Apply filter to input only (no adaptation) */
    double filterOnly(double input) const;

    /** @brief System identification: adapt to unknown system */
    QVector<FilterOutput> systemIdentify(const QVector<double>& input,
                                          const QVector<double>& systemOutput);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sampleProcessed(double error, double weightNorm);
    void blockDone(int length, double avgError, double timeMs);

private:
    FilterConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_errorSum = 0.0;

    QVector<double> m_w;          // Filter weights (N x 1)
    QVector<QVector<double>> m_P; // Inverse correlation matrix (N x N)
    QVector<double> m_xBuf;       // Input buffer (circular)
    int m_bufIdx = 0;

    /** @brief Initialize filter state */
    void initFilter();

    /** @brief Get input vector from circular buffer */
    QVector<double> getInputVector() const;

    /** @brief RLS update step */
    void rlsUpdate(const QVector<double>& x, double error);
};
