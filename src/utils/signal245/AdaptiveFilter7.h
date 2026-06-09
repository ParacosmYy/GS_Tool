/**
 * @file AdaptiveFilter7.h
 * @brief 自适应滤波器(归一化LMS+梯度估计变步长收敛跟踪) — Adaptive Filter with Normalized LMS and Variable Step-Size Using Gradient Estimation for Convergence Tracking
 *
 * 功能: 实现自适应滤波器(adaptive filter)，采用归一化LMS(NLMS)算法更新权重，
 *       使用梯度估计(gradient estimation)动态调整步长(variable step-size)，
 *       实现收敛速度与稳态误差的自动平衡跟踪。
 *
 * 协作: Compressor6(动态范围压缩) / KalmanFilter8(卡尔曼滤波) / WienerFilter5(维纳滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 自适应滤波器(归一化LMS+梯度估计变步长收敛跟踪)
 */
class AdaptiveFilter7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterOrder = 0;
        int numSamples = 0;
        double finalMu = 0.0;
        double finalErrorPower = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AdaptiveFilter7(QObject *parent = nullptr);
    ~AdaptiveFilter7() override;

    /** @brief Set filter order (number of taps) */
    void setFilterOrder(int order);

    /** @brief Set initial step-size mu */
    void setInitialMu(double mu);

    /** @brief Set minimum step-size */
    void setMuMin(double mu);

    /** @brief Set maximum step-size */
    void setMuMax(double mu);

    /** @brief Set step-size adaptation rate */
    void setMuAdaptRate(double rate);

    /** @brief Process input/desired pair, returns filter output */
    QVector<double> process(const QVector<double>& input,
                            const QVector<double>& desired);

    /** @brief Get current filter coefficients */
    QVector<double> coefficients() const;

    /** @brief Get error signal history */
    QVector<double> errorHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterUpdated(int sampleIdx, double error, double mu, double timeMs);

private:
    int m_order = 32;
    double m_mu = 0.1;
    double m_muMin = 0.001;
    double m_muMax = 1.0;
    double m_muAdaptRate = 0.01;

    QVector<double> m_weights;
    QVector<double> m_delayLine;
    int m_delayPos = 0;

    QVector<double> m_errorHistory;
    double m_prevError = 0.0;
    double m_gradEstimate = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Push sample into circular delay line */
    void pushSample(double sample);

    /** @brief Compute dot product of delay line with weights */
    double dotProduct() const;

    /** @brief Update step-size using gradient estimation */
    void updateStepSize(double error);

    /** @brief NLMS weight update */
    void updateWeights(double error);
};
