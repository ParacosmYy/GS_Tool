/**
 * @file UnscentedKalmanFilter.h
 * @brief 无迹卡尔曼滤波器(UKF) — 无需Jacobian的非线性状态估计
 *
 * 功能: UKF通过Sigma点采样处理非线性系统，无需计算Jacobian，
 *       支持状态/协方差预测更新，统计滤波步数/耗时。
 */
#ifndef UNSCENTEDKALMANFILTER_H
#define UNSCENTEDKALMANFILTER_H

#include <QObject>
#include <QVector>
#include <functional>

class UnscentedKalmanFilter : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalPredictions = 0;
        quint64 totalUpdates = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    /** 状态转移函数: f(state) → predicted_state */
    using StateFunc = std::function<QVector<double>(const QVector<double>&)>;
    /** 观测函数: h(state) → observation */
    using ObsFunc = std::function<QVector<double>(const QVector<double>&)>;

    explicit UnscentedKalmanFilter(int stateDim, int obsDim,
                                    QObject* parent = nullptr);

    /** @brief 设置状态转移函数 */
    void setStateFunc(StateFunc f);
    /** @brief 设置观测函数 */
    void setObsFunc(ObsFunc h);

    /** @brief 设置过程噪声协方差 */
    void setProcessNoise(const QVector<QVector<double>>& Q);
    /** @brief 设置观测噪声协方差 */
    void setMeasurementNoise(const QVector<QVector<double>>& R);

    /** @brief 设置UKF参数 @param alpha 扩展参数 @param beta 分布参数 @param kappa 二次参数 */
    void setParameters(double alpha = 1e-3, double beta = 2.0, double kappa = 0.0);

    /** @brief 初始化状态 */
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
    void generateSigmaPoints();
    QVector<double> weightedMean(const QVector<QVector<double>>& points) const;
    QVector<QVector<double>> weightedCovariance(
        const QVector<QVector<double>>& points,
        const QVector<double>& mean) const;

    int m_stateDim;
    int m_obsDim;
    int m_numSigma;
    QVector<double> m_state;
    QVector<QVector<double>> m_P;
    QVector<QVector<double>> m_Q;
    QVector<QVector<double>> m_R;
    StateFunc m_f;
    ObsFunc m_h;
    bool m_initialized;

    double m_alpha, m_beta, m_kappa;
    double m_lambda;
    QVector<double> m_wm;  ///< 均值权重
    QVector<double> m_wc;  ///< 协方差权重
    QVector<QVector<double>> m_sigmaPoints;

    Stats m_stats;
    double m_timeSum;
};

#endif // UNSCENTEDKALMANFILTER_H
