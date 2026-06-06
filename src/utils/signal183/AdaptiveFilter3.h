/**
 * @file AdaptiveFilter3.h
 * @brief 自适应滤波器(LMS/NLMS/RLS可变步长+收敛监控) — Adaptive LMS/NLMS/RLS Filter with Variable Step-size and Convergence Monitoring
 *
 * 功能: 实现自适应滤波器，支持LMS/NLMS/RLS三种算法、可变步长控制和
 *       收敛状态监控。
 *
 * 协作: WienerFilter2(维纳滤波) / KalmanFilter3(卡尔曼) / LMSFilter4(LMS滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 自适应滤波器(LMS/NLMS/RLS+可变步长)
 */
class AdaptiveFilter3 : public QObject {
    Q_OBJECT

public:
    /** @brief 算法类型 */
    enum Algorithm {
        LMS,    ///< Least Mean Squares
        NLMS,   ///< Normalized LMS
        RLS     ///< Recursive Least Squares
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;
        int filterOrder = 0;
        int numIterations = 0;
        double finalError = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AdaptiveFilter3(QObject *parent = nullptr);
    ~AdaptiveFilter3() override;

    void setFilterOrder(int order);
    void setStepSize(double mu);
    void setAlgorithm(Algorithm algo);
    void setRLSLambda(double lambda);
    void setRLSDelta(double delta);
    void setVariableStepSize(bool enabled);
    void setStepSizeRange(double muMin, double muMax);

    /** @brief 处理单帧(输入+期望)，返回输出 */
    double processSample(double input, double desired);

    /** @brief 批量处理 */
    QVector<double> processBatch(const QVector<double>& input,
                                  const QVector<double>& desired);

    /** @brief 获取当前滤波器系数 */
    QVector<double> coefficients() const;

    /** @brief 获取误差历史 */
    QVector<double> errorHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();
    void resetFilter();

signals:
    void filterConverged(int iteration, double error);
    void processingCompleted(int samples, double finalError);

private:
    int m_order = 32;
    double m_mu = 0.01;
    Algorithm m_algo = NLMS;
    double m_lambda = 0.999;  ///< RLS forgetting factor
    double m_delta = 0.01;    ///< RLS initialization
    bool m_variableStep = false;
    double m_muMin = 1e-6;
    double m_muMax = 0.1;

    QVector<double> m_weights;  ///< Filter coefficients
    QVector<double> m_delayLine; ///< Input delay line
    QVector<double> m_errorHistory;

    // RLS specific
    QVector<QVector<double>> m_P; ///< Inverse correlation matrix

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_prevError = 0.0;

    /** @brief LMS update */
    double updateLMS(double input, double desired);

    /** @brief NLMS update */
    double updateNLMS(double input, double desired);

    /** @brief RLS update */
    double updateRLS(double input, double desired);

    /** @brief Update delay line */
    void pushSample(double sample);

    /** @brief Compute filter output */
    double computeOutput() const;

    /** @brief Variable step size (if enabled) */
    void adaptStepSize(double error);
};
