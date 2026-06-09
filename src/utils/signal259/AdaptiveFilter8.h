/**
 * @file AdaptiveFilter8.h
 * @brief 自适应滤波器(RLS+QR分解数值稳定快速RLS) — Adaptive Filter with Recursive Least Squares and QR Decomposition for Numerically Stable Fast RLS
 *
 * 功能: 实现自适应滤波器(Adaptive Filter)，采用递归最小二乘(RLS)算法
 *       配合QR分解(QR decomposition)实现数值稳定的快速RLS(Numerically
 *       Stable Fast RLS)，用于系统辨识和噪声消除。
 *
 * 协作: WienerFilter5(维纳滤波) / KalmanFilter6(卡尔曼滤波) / LMSFilter3(LMS滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 自适应滤波器(RLS+QR分解数值稳定快速RLS)
 */
class AdaptiveFilter8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterOrder = 0;
        int samplesProcessed = 0;
        double finalError = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Filter configuration */
    struct Config {
        int order = 32;
        double forgettingFactor = 0.99;
        double regularization = 1e-6;
        bool useQR = true;       // Use QR-based RLS
    };

    explicit AdaptiveFilter8(QObject *parent = nullptr);
    ~AdaptiveFilter8() override;

    /** @brief Configure filter parameters */
    void configure(const Config& config);

    /** @brief Set filter order */
    void setOrder(int order);

    /** @brief Set forgetting factor (0 < lambda <= 1) */
    void setForgettingFactor(double lambda);

    /** @brief Set regularization parameter */
    void setRegularization(double delta);

    /** @brief Enable QR-based numerically stable mode */
    void setQRMode(bool enabled);

    /** @brief Process one sample: input + desired -> output + error */
    double processSample(double input, double desired);

    /** @brief Process a block of samples */
    QVector<double> processBlock(const QVector<double>& input,
                                 const QVector<double>& desired);

    /** @brief Get current filter coefficients */
    QVector<double> coefficients() const;

    /** @brief Get learning curve (squared errors) */
    QVector<double> learningCurve() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterUpdated(int order, double error, double timeMs);

private:
    Config m_config;
    int m_order = 32;

    // Standard RLS state
    QVector<QVector<double>> m_P;    // Inverse correlation matrix
    QVector<double> m_w;              // Filter weights
    QVector<double> m_xBuf;           // Input buffer (tapped delay line)
    int m_bufIdx = 0;

    // QR-RLS state
    QVector<QVector<double>> m_R;     // Upper triangular from QR
    QVector<double> m_errQR;

    QVector<double> m_errors;         // Learning curve
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Standard RLS update step */
    double rlsUpdate(double input, double desired);

    /** @brief QR-based RLS update step */
    double qrRLSUpdate(double input, double desired);

    /** @brief Givens rotation for QR update */
    void givensRotate(double a, double b, double& c, double& s) const;

    /** @brief Initialize filter state */
    void initFilter();
};
