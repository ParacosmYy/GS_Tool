/**
 * @file UnscentedKalmanFilter.cpp
 * @brief 无迹卡尔曼滤波器(UKF) — Sigma点采样非线性估计
 */

#include "UnscentedKalmanFilter.h"
#include <QElapsedTimer>
#include <cmath>

UnscentedKalmanFilter::UnscentedKalmanFilter(int stateDim, int obsDim,
                                               QObject* parent)
    : QObject(parent)
    , m_stateDim(stateDim)
    , m_obsDim(obsDim)
    , m_numSigma(2 * stateDim + 1)
    , m_state(stateDim, 0.0)
    , m_P(stateDim, QVector<double>(stateDim, 0.0))
    , m_Q(stateDim, QVector<double>(stateDim, 0.0))
    , m_R(obsDim, QVector<double>(obsDim, 0.0))
    , m_initialized(false)
    , m_alpha(1e-3)
    , m_beta(2.0)
    , m_kappa(0.0)
    , m_lambda(0.0)
    , m_wm(m_numSigma, 0.0)
    , m_wc(m_numSigma, 0.0)
    , m_sigmaPoints(m_numSigma, QVector<double>(stateDim, 0.0))
    , m_timeSum(0.0)
{
    setParameters(1e-3, 2.0, 0.0);
    for (int i = 0; i < stateDim; ++i) m_P[i][i] = 1.0;
    for (int i = 0; i < stateDim; ++i) m_Q[i][i] = 1e-4;
    for (int i = 0; i < obsDim; ++i) m_R[i][i] = 1e-2;
}

void UnscentedKalmanFilter::setStateFunc(StateFunc f) { m_f = f; }
void UnscentedKalmanFilter::setObsFunc(ObsFunc h) { m_h = h; }
void UnscentedKalmanFilter::setProcessNoise(const QVector<QVector<double>>& Q) { m_Q = Q; }
void UnscentedKalmanFilter::setMeasurementNoise(const QVector<QVector<double>>& R) { m_R = R; }

void UnscentedKalmanFilter::setParameters(double alpha, double beta, double kappa)
{
    m_alpha = alpha;
    m_beta = beta;
    m_kappa = kappa;
    m_lambda = alpha * alpha * (m_stateDim + kappa) - m_stateDim;

    m_wm[0] = m_lambda / (m_stateDim + m_lambda);
    m_wc[0] = m_lambda / (m_stateDim + m_lambda) + (1.0 - alpha * alpha + beta);
    for (int i = 1; i < m_numSigma; ++i) {
        m_wm[i] = 1.0 / (2.0 * (m_stateDim + m_lambda));
        m_wc[i] = m_wm[i];
    }
}

void UnscentedKalmanFilter::initialize(const QVector<double>& state,
                                          const QVector<QVector<double>>& covariance)
{
    m_state = state;
    m_P = covariance;
    m_initialized = true;
}

void UnscentedKalmanFilter::generateSigmaPoints()
{
    int n = m_stateDim;
    double scale = n + m_lambda;

    /* 计算P的Cholesky分解(简化: 对角近似) */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        L[i][i] = std::sqrt(std::max(scale * m_P[i][i], 0.0));
    }

    m_sigmaPoints[0] = m_state;
    for (int i = 0; i < n; ++i) {
        m_sigmaPoints[i + 1] = m_state;
        m_sigmaPoints[n + i + 1] = m_state;
        for (int j = 0; j < n; ++j) {
            m_sigmaPoints[i + 1][j] += L[j][i];
            m_sigmaPoints[n + i + 1][j] -= L[j][i];
        }
    }
}

QVector<double> UnscentedKalmanFilter::weightedMean(
    const QVector<QVector<double>>& points) const
{
    int n = points[0].size();
    QVector<double> mean(n, 0.0);
    for (int i = 0; i < m_numSigma; ++i) {
        for (int j = 0; j < n; ++j) {
            mean[j] += m_wm[i] * points[i][j];
        }
    }
    return mean;
}

QVector<QVector<double>> UnscentedKalmanFilter::weightedCovariance(
    const QVector<QVector<double>>& points,
    const QVector<double>& mean) const
{
    int n = mean.size();
    QVector<QVector<double>> cov(n, QVector<double>(n, 0.0));
    for (int i = 0; i < m_numSigma; ++i) {
        QVector<double> diff(n);
        for (int j = 0; j < n; ++j) diff[j] = points[i][j] - mean[j];
        for (int a = 0; a < n; ++a)
            for (int b = 0; b < n; ++b)
                cov[a][b] += m_wc[i] * diff[a] * diff[b];
    }
    return cov;
}

void UnscentedKalmanFilter::predict()
{
    if (!m_initialized || !m_f) return;

    QElapsedTimer timer;
    timer.start();

    generateSigmaPoints();

    /* 传播Sigma点 */
    QVector<QVector<double>> sigmaPred(m_numSigma, QVector<double>(m_stateDim));
    for (int i = 0; i < m_numSigma; ++i) {
        sigmaPred[i] = m_f(m_sigmaPoints[i]);
    }

    m_state = weightedMean(sigmaPred);
    m_P = weightedCovariance(sigmaPred, m_state);

    /* 加过程噪声 */
    for (int i = 0; i < m_stateDim; ++i)
        for (int j = 0; j < m_stateDim; ++j)
            m_P[i][j] += m_Q[i][j];

    m_stats.totalPredictions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalPredictions + m_stats.totalUpdates);

    emit statePredicted(m_state);
}

void UnscentedKalmanFilter::update(const QVector<double>& measurement)
{
    if (!m_initialized || !m_h) return;

    QElapsedTimer timer;
    timer.start();

    /* 传播观测Sigma点 */
    QVector<QVector<double>> zSigma(m_numSigma, QVector<double>(m_obsDim));
    for (int i = 0; i < m_numSigma; ++i) {
        zSigma[i] = m_h(m_sigmaPoints[i]);
    }

    QVector<double> zPred = weightedMean(zSigma);
    QVector<QVector<double>> Pz = weightedCovariance(zSigma, zPred);

    /* 加观测噪声 */
    for (int i = 0; i < m_obsDim; ++i)
        for (int j = 0; j < m_obsDim; ++j)
            Pz[i][j] += m_R[i][j];

    /* 交叉协方差 Pxz */
    QVector<QVector<double>> Pxz(m_stateDim, QVector<double>(m_obsDim, 0.0));
    for (int i = 0; i < m_numSigma; ++i) {
        QVector<double> dx(m_stateDim), dz(m_obsDim);
        for (int j = 0; j < m_stateDim; ++j) dx[j] = m_sigmaPoints[i][j] - m_state[j];
        for (int j = 0; j < m_obsDim; ++j) dz[j] = zSigma[i][j] - zPred[j];
        for (int a = 0; a < m_stateDim; ++a)
            for (int b = 0; b < m_obsDim; ++b)
                Pxz[a][b] += m_wc[i] * dx[a] * dz[b];
    }

    /* K = Pxz * Pz^-1 */
    /* 简化: Pz对角近似求逆 */
    QVector<QVector<double>> PzInv(m_obsDim, QVector<double>(m_obsDim, 0.0));
    for (int i = 0; i < m_obsDim; ++i) {
        PzInv[i][i] = (std::abs(Pz[i][i]) > 1e-15) ? 1.0 / Pz[i][i] : 0.0;
    }

    QVector<QVector<double>> K(m_stateDim, QVector<double>(m_obsDim, 0.0));
    for (int i = 0; i < m_stateDim; ++i)
        for (int j = 0; j < m_obsDim; ++j)
            for (int k = 0; k < m_obsDim; ++k)
                K[i][j] += Pxz[i][k] * PzInv[k][j];

    /* x = x + K * (z - zPred) */
    QVector<double> innovation(m_obsDim);
    for (int i = 0; i < m_obsDim; ++i) innovation[i] = measurement[i] - zPred[i];

    for (int i = 0; i < m_stateDim; ++i)
        for (int j = 0; j < m_obsDim; ++j)
            m_state[i] += K[i][j] * innovation[j];

    /* P = P - K * Pz * K^T */
    for (int i = 0; i < m_stateDim; ++i)
        for (int j = 0; j < m_stateDim; ++j)
            for (int a = 0; a < m_obsDim; ++a)
                for (int b = 0; b < m_obsDim; ++b)
                    m_P[i][j] -= K[i][a] * Pz[a][b] * K[j][b];

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalPredictions + m_stats.totalUpdates);

    emit stateUpdated(m_state);
}

void UnscentedKalmanFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
