/**
 * @file ExtendedKalmanFilter.cpp
 * @brief 扩展卡尔曼滤波器(EKF) — 非线性状态估计
 */

#include "ExtendedKalmanFilter.h"
#include <QElapsedTimer>
#include <cmath>

ExtendedKalmanFilter::ExtendedKalmanFilter(int stateDim, int obsDim,
                                             QObject* parent)
    : QObject(parent)
    , m_stateDim(stateDim)
    , m_obsDim(obsDim)
    , m_state(stateDim, 0.0)
    , m_P(stateDim, QVector<double>(stateDim, 0.0))
    , m_Q(stateDim, QVector<double>(stateDim, 0.0))
    , m_R(obsDim, QVector<double>(obsDim, 0.0))
    , m_initialized(false)
    , m_timeSum(0.0)
{
    /* P = I */
    for (int i = 0; i < stateDim; ++i) m_P[i][i] = 1.0;
}

void ExtendedKalmanFilter::setStateFunc(StateFunc f, JacobianFunc jacobianF)
{
    m_f = f;
    m_jacobianF = jacobianF;
}

void ExtendedKalmanFilter::setObsFunc(ObsFunc h, JacobianFunc jacobianH)
{
    m_h = h;
    m_jacobianH = jacobianH;
}

void ExtendedKalmanFilter::setProcessNoise(const QVector<QVector<double>>& Q)
{
    m_Q = Q;
}

void ExtendedKalmanFilter::setMeasurementNoise(const QVector<QVector<double>>& R)
{
    m_R = R;
}

void ExtendedKalmanFilter::initialize(const QVector<double>& state,
                                        const QVector<QVector<double>>& covariance)
{
    m_state = state;
    m_P = covariance;
    m_initialized = true;
}

void ExtendedKalmanFilter::predict()
{
    if (!m_initialized || !m_f || !m_jacobianF) return;

    QElapsedTimer timer;
    timer.start();

    /* x_pred = f(x) */
    m_state = m_f(m_state);

    /* F = df/dx */
    QVector<QVector<double>> F = m_jacobianF(m_state);

    /* P = F * P * F^T + Q */
    QVector<QVector<double>> FPFt = matMul(matMul(F, m_P), matTranspose(F));
    m_P = matAdd(FPFt, m_Q);

    m_stats.totalPredictions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalPredictions + m_stats.totalUpdates);

    emit statePredicted(m_state);
}

void ExtendedKalmanFilter::update(const QVector<double>& measurement)
{
    if (!m_initialized || !m_h || !m_jacobianH) return;

    QElapsedTimer timer;
    timer.start();

    /* y = z - h(x) */
    QVector<double> z_pred = m_h(m_state);
    QVector<double> y(m_obsDim);
    for (int i = 0; i < m_obsDim; ++i) y[i] = measurement[i] - z_pred[i];

    /* H = dh/dx */
    QVector<QVector<double>> H = m_jacobianH(m_state);

    /* S = H * P * H^T + R */
    QVector<QVector<double>> S = matAdd(matMul(matMul(H, m_P), matTranspose(H)), m_R);

    /* K = P * H^T * S^-1 */
    QVector<QVector<double>> Sinv = matInverse(S);
    QVector<QVector<double>> K = matMul(matMul(m_P, matTranspose(H)), Sinv);

    /* x = x + K * y */
    for (int i = 0; i < m_stateDim; ++i) {
        for (int j = 0; j < m_obsDim; ++j) {
            m_state[i] += K[i][j] * y[j];
        }
    }

    /* P = (I - K * H) * P */
    QVector<QVector<double>> I_KH = matSub(matIdentity(m_stateDim), matMul(K, H));
    m_P = matMul(I_KH, m_P);

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalPredictions + m_stats.totalUpdates);

    emit stateUpdated(m_state);
}

QVector<QVector<double>> ExtendedKalmanFilter::matMul(
    const QVector<QVector<double>>& A, const QVector<QVector<double>>& B) const
{
    int m = A.size();
    int n = B.isEmpty() ? 0 : B[0].size();
    int p = B.size();
    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < p; ++k)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

QVector<QVector<double>> ExtendedKalmanFilter::matTranspose(
    const QVector<QVector<double>>& A) const
{
    if (A.isEmpty()) return {};
    int m = A.size(), n = A[0].size();
    QVector<QVector<double>> T(n, QVector<double>(m));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            T[j][i] = A[i][j];
    return T;
}

QVector<QVector<double>> ExtendedKalmanFilter::matAdd(
    const QVector<QVector<double>>& A, const QVector<QVector<double>>& B) const
{
    int m = A.size(), n = A.isEmpty() ? 0 : A[0].size();
    QVector<QVector<double>> C(m, QVector<double>(n));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

QVector<QVector<double>> ExtendedKalmanFilter::matSub(
    const QVector<QVector<double>>& A, const QVector<QVector<double>>& B) const
{
    int m = A.size(), n = A.isEmpty() ? 0 : A[0].size();
    QVector<QVector<double>> C(m, QVector<double>(n));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] - B[i][j];
    return C;
}

QVector<QVector<double>> ExtendedKalmanFilter::matIdentity(int n) const
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

QVector<QVector<double>> ExtendedKalmanFilter::matInverse(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    if (n == 0) return {};

    /* 增广矩阵 [A|I] — Gauss-Jordan */
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = A[i][j];
        aug[i][n + i] = 1.0;
    }

    for (int col = 0; col < n; ++col) {
        /* 主元选取 */
        int maxRow = col;
        for (int row = col + 1; row < n; ++row) {
            if (std::abs(aug[row][col]) > std::abs(aug[maxRow][col])) maxRow = row;
        }
        std::swap(aug[col], aug[maxRow]);

        double pivot = aug[col][col];
        if (std::abs(pivot) < 1e-15) continue;

        for (int j = 0; j < 2 * n; ++j) aug[col][j] /= pivot;

        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j) aug[row][j] -= factor * aug[col][j];
        }
    }

    QVector<QVector<double>> inv(n, QVector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            inv[i][j] = aug[i][n + j];
    return inv;
}

void ExtendedKalmanFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
