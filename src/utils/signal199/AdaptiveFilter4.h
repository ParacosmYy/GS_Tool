/**
 * @file AdaptiveFilter4.h
 * @brief 仿射投影算法(APA投影阶数控制+正则化) — Affine Projection Algorithm with Projection Order Control and Regularization
 *
 * 功能: 实现仿射投影自适应滤波器，支持投影阶数控制、
 *       正则化和收敛性优化。
 *
 * 协作: AdaptiveFilter3(NLMS) / LMS8(LMS滤波器) / RLS5(RLS滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 仿射投影算法(投影阶数控制+正则化)
 */
class AdaptiveFilter4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;
        int filterLength = 0;
        int projectionOrder = 0;
        double misalignment = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AdaptiveFilter4(QObject *parent = nullptr);
    ~AdaptiveFilter4() override;

    void setFilterLength(int len);
    void setStepSize(double mu);
    void setProjectionOrder(int p);
    void setRegularization(double delta);
    void setAdaptiveOrder(bool enable);

    /** @brief Process one sample: returns filter output */
    double process(double desired, double input);

    /** @brief Process batch of samples */
    QVector<double> processBatch(const QVector<double>& desired,
                                  const QVector<double>& input);

    /** @brief Get current filter coefficients */
    QVector<double> coefficients() const;

    /** @brief Get current projection order */
    int currentProjectionOrder() const;

    /** @brief Get filter output for input without updating */
    double filterOutput(double input) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();
    void resetFilter();

signals:
    void filterUpdated(int iteration, double error, double misalignment);

private:
    int m_filterLen = 32;
    double m_stepSize = 0.1;
    int m_projOrder = 4;
    double m_regularization = 1e-6;
    bool m_adaptiveOrder = true;

    // Filter state
    QVector<double> m_weights;     // filter coefficients
    QVector<double> m_inputBuf;    // circular input buffer
    int m_bufIdx = 0;

    // APA matrix buffers (P x filterLen)
    QVector<QVector<double>> m_inputMatrix;  // projection matrix
    QVector<double> m_errorVec;              // error vector
    int m_matIdx = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleCount = 0;

    /** @brief Auto-tune projection order based on convergence */
    int autoProjectionOrder(double errorPower);

    /** @brief Solve normal equations with regularization */
    QVector<double> solveRegularized(const QVector<QVector<double>>& U,
                                      const QVector<double>& e) const;
};
