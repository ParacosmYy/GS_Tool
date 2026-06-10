/**
 * @file AdaptiveFilter9.h
 * @brief 自适应滤波器(归一化LMS与梯度估计变步长时变系统辨识) — Adaptive Filter with Normalized LMS and Variable Step-Size via Gradient Estimation for Time-Varying System Identification
 *
 * 功能: 实现自适应滤波器(adaptive filter)，采用归一化LMS(normalized LMS)
 *       与梯度估计变步长(gradient estimation variable step-size)实现时变系统辨识(time-varying system identification)。
 *
 * 协作: WienerFilter8(维纳滤波) / KalmanFilter7(卡尔曼) / LSLFilter6(格型滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 自适应滤波器(归一化LMS与梯度估计变步长时变系统辨识)
 */
class AdaptiveFilter9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterOrder = 0;
        int blockSize = 0;
        double finalErrorPower = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AdaptiveFilter9(QObject *parent = nullptr);
    ~AdaptiveFilter9() override;

    /** @brief Set filter order (number of taps) */
    void setFilterOrder(int order);

    /** @brief Set initial step size mu for NLMS */
    void setStepSize(double mu);

    /** @brief Set regularization factor to prevent division by zero */
    void setRegularization(double eps);

    /** @brief Enable variable step-size mode via gradient estimation */
    void setVariableStepSize(bool enabled, double muMin = 1e-6, double muMax = 0.1,
                              double nu = 0.05);

    /** @brief Process input/desired pair, returns filter output (sample-by-sample) */
    QVector<double> process(const QVector<double>& input, const QVector<double>& desired);

    /** @brief Get current filter coefficients */
    QVector<double> coefficients() const;

    /** @brief Get error signal from last processing */
    QVector<double> errors() const;

    /** @brief Get learning curve (MSE history) */
    QVector<double> learningCurve() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filteringDone(int filterOrder, int blockSize, double finalError, double timeMs);

private:
    int m_order = 32;
    double m_mu = 0.01;
    double m_eps = 1e-8;
    bool m_variableStep = false;
    double m_muMin = 1e-6;
    double m_muMax = 0.1;
    double m_nu = 0.05;

    QVector<double> m_weights;
    QVector<double> m_inputBuf;  // circular buffer for tap delay line
    int m_bufPos = 0;

    QVector<double> m_errors;
    QVector<double> m_learningCurve;

    // Variable step-size state
    double m_currentMu = 0.01;
    double m_gradEst = 0.0;
    double m_prevError = 0.0;
    double m_prevGrad = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get delayed sample from circular buffer */
    double getDelayedSample(int delay) const;

    /** @brief Compute filter output for current tap inputs */
    double computeOutput() const;

    /** @brief Update weights using NLMS */
    void updateWeights(double error, double power);

    /** @brief Update step-size using gradient estimation */
    void updateStepSize(double error);
};
