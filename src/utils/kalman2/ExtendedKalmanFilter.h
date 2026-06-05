/**
 * @file ExtendedKalmanFilter.h
 * @brief 扩展卡尔曼滤波器(EKF) — 非线性状态估计
 *
 * 功能: 扩展卡尔曼滤波器，支持非线性状态转移/观测模型、
 *       Jacobian计算、状态/协方差预测更新，统计滤波步数/耗时。
 */
#ifndef EXTENDEDKALMANFILTER_H
#define EXTENDEDKALMANFILTER_H

#include <QObject>
#include <QVector>

class ExtendedKalmanFilter : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalPredictions = 0;
        quint64 totalUpdates = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    /** 状态转移函数类型: f(state) → predicted_state */
    using StateFunc = std::function<QVector<double>(const QVector<double>&)>;
    /** 观测函数类型: h(state) → observation */
    using ObsFunc = std::function<QVector<double>(const QVector<double>&)>;
    /** Jacobian类型: df/dx */
    using JacobianFunc = std::function<QVector<QVector<double>>(const QVector<double>&)>;

    explicit ExtendedKalmanFilter(int stateDim, int obsDim,
                                   QObject* parent = nullptr);

    /** @brief 设置状态转移函数和Jacobian */
    void setStateFunc(StateFunc f, JacobianFunc jacobianF);

    /** @brief 设置观测函数和Jacobian */
    void setObsFunc(ObsFunc h, JacobianFunc jacobianH);

    /** @brief 设置过程噪声协方差 @param Q 协方差矩阵 */
    void setProcessNoise(const QVector<QVector<double>>& Q);

    /** @brief 设置观测噪声协方差 @param R 协方差矩阵 */
    void setMeasurementNoise(const QVector<QVector<double>>& R);

    /** @brief 初始化状态 @param state 初始状态 @param covariance 初始协方差 */
    void initialize(const QVector<double>& state,
                    const QVector<QVector<double>>& covariance);

    /** @brief 预测步骤 */
    void predict();

    /** @brief 更新步骤 @param measurement 观测值 */
    void update(const QVector<double>& measurement);

    QVector<double> state() const { return m_state; }
    QVector<QVector<double>> covariance() const { return m_P; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void statePredicted(const QVector<double>& state);
    void stateUpdated(const QVector<double>& state);

private:
    QVector<QVector<double>> matMul(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B) const;
    QVector<QVector<double>> matTranspose(
        const QVector<QVector<double>>& A) const;
    QVector<QVector<double>> matAdd(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B) const;
    QVector<QVector<double>> matSub(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B) const;
    QVector<QVector<double>> matIdentity(int n) const;
    QVector<QVector<double>> matInverse(
        const QVector<QVector<double>>& A) const;

    int m_stateDim;
    int m_obsDim;
    QVector<double> m_state;
    QVector<QVector<double>> m_P;
    QVector<QVector<double>> m_Q;
    QVector<QVector<double>> m_R;
    StateFunc m_f;
    ObsFunc m_h;
    JacobianFunc m_jacobianF;
    JacobianFunc m_jacobianH;
    bool m_initialized;
    Stats m_stats;
    double m_timeSum;
};

#endif // EXTENDEDKALMANFILTER_H
