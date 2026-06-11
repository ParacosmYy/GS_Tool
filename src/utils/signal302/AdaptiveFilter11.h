/**
 * @file AdaptiveFilter11.h
 * @brief 自适应滤波器(仿射投影算法与正则化矩阵求逆实现快速跟踪多径信道均衡) — Adaptive Filter with Affine Projection Algorithm and Regularized Matrix Inversion for Fast-Tracking Multi-Path Channel Equalization
 *
 * 功能: 实现自适应滤波器(adaptive filter)，采用仿射投影算法(affine projection algorithm)
 *       与正则化矩阵求逆(regularized matrix inversion)实现快速跟踪多径信道均衡(fast-tracking multi-path channel equalization)。
 *
 * 协作: LMSFilter(LMS滤波器) / RLFilter(递归最小二乘) / WienerFilter(维纳滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

class AdaptiveFilter11 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter state */
    struct FilterState {
        QVector<double> weights;
        double errorPower = 0.0;
        double misalignment = 0.0;
        int iteration = 0;
    };

    /** @brief Processing result */
    struct FilterResult {
        QVector<double> output;         // filter output
        QVector<double> error;          // error signal
        QVector<double> weightTrace;    // weight norm history
        double finalErrorPower = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalBlocks = 0;
        int filterOrder = 0;
        int projectionOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AdaptiveFilter11(QObject *parent = nullptr);
    ~AdaptiveFilter11() override;

    void setFilterOrder(int order);
    void setProjectionOrder(int order);
    void setStepSize(double mu);
    void setRegularization(double lambda);

    /** @brief Process input/desired signal pair through APA adaptive filter */
    FilterResult process(const QVector<double>& input, const QVector<double>& desired);

    /** @brief Get current filter weights */
    const QVector<double>& weights() const { return m_weights; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterDone(int iterations, double errorPower, double timeMs);

private:
    int m_filterOrder = 32;
    int m_projectionOrder = 4;
    double m_stepSize = 0.1;
    double m_regularization = 1e-6;
    QVector<double> m_weights;
    QVector<QVector<double>> m_inputMatrix;  // projection order x filter order
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Solve regularized system (X^T*X + lambda*I) * w = X^T * d via Cholesky */
    QVector<double> solveRegularized(const QVector<QVector<double>>& X,
                                      const QVector<double>& d) const;

    /** @brief Push new input sample into sliding input matrix */
    void shiftInputMatrix(double newSample);

    /** @brief Compute filter output for given input vector */
    double computeOutput(const QVector<double>& inputVec) const;
};
