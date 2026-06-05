/**
 * @file KalmanFilter2.cpp
 * @brief 扩展卡尔曼滤波(EKF)实现 — 非线性状态估计与雅可比计算
 */

#include "KalmanFilter2.h"

#include <QElapsedTimer>
#include <cmath>

/* ---------- 构造函数 ---------- */

KalmanFilter2::KalmanFilter2(QObject* parent)
    : QObject(parent)
{
}

/* ---------- 初始化 ---------- */

void KalmanFilter2::initialize(const EKFParams& params,
                                 const QVector<double>& initialState,
                                 const Matrix& initialCovariance)
{
    m_stateDim = params.stateDim;
    m_obsDim = params.obsDim;
    m_processNoise = params.processNoise;
    m_measNoise = params.measurementNoise;
    m_stateFunc = params.stateFunc;
    m_obsFunc = params.obsFunc;
    m_state = initialState;
    m_covariance = initialCovariance;
    m_innovation = QVector<double>(m_obsDim, 0.0);
    m_initialized = true;
}

/* ---------- 预测步骤 ---------- */

void KalmanFilter2::predict(const QVector<double>& controlInput)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) return;

    /* 状态预测: x_pred = f(x) */
    QVector<double> xPred = m_stateFunc(m_state);

    /* 加入控制输入(若有) */
    if (controlInput.size() == m_stateDim) {
        for (int i = 0; i < m_stateDim; ++i) {
            xPred[i] += controlInput[i];
        }
    }

    /* 计算雅可比 F = df/dx */
    Matrix F = computeStateJacobian(m_state);
    Matrix FT = matTranspose(F);

    /* 协方差预测: P_pred = F * P * F^T + Q */
    Matrix PFPt = matMul(matMul(F, m_covariance), FT);
    m_covariance = matAdd(PFPt, m_processNoise);

    m_state = xPred;

    m_stats.totalPredicts++;
    m_stats.totalJacobians++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalPredicts + m_stats.totalUpdates;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / totalOps : 0.0;

    emit predicted(m_state);
}

/* ---------- 更新步骤 ---------- */

void KalmanFilter2::update(const QVector<double>& measurement)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) return;

    /* 观测预测: z_pred = h(x) */
    QVector<double> zPred = m_obsFunc(m_state);

    /* 创新向量: y = z - z_pred */
    m_innovation = QVector<double>(m_obsDim);
    for (int i = 0; i < m_obsDim; ++i) {
        m_innovation[i] = measurement[i] - zPred[i];
    }

    /* 计算观测雅可比 H = dh/dx */
    Matrix H = computeObsJacobian(m_state);
    Matrix HT = matTranspose(H);

    /* 创新(残差)协方差: S = H * P * H^T + R */
    Matrix PHT = matMul(m_covariance, HT);
    Matrix HPHt = matMul(H, PHT);
    Matrix S = matAdd(HPHt, m_measNoise);

    /* 卡尔曼增益: K = P * H^T * S^{-1} */
    Matrix Sinv = matInverse(S);
    Matrix K = matMul(PHT, Sinv);

    /* 状态更新: x = x + K * y */
    for (int i = 0; i < m_stateDim; ++i) {
        double gainY = 0.0;
        for (int j = 0; j < m_obsDim; ++j) {
            gainY += K[i][j] * m_innovation[j];
        }
        m_state[i] += gainY;
    }

    /* 协方差更新: P = (I - K * H) * P */
    Matrix KH = matMul(K, H);
    Matrix I_KH = matSub(identity(m_stateDim), KH);
    m_covariance = matMul(I_KH, m_covariance);

    m_stats.totalUpdates++;
    m_stats.totalJacobians++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalPredicts + m_stats.totalUpdates;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / totalOps : 0.0;

    double innNorm = vecNorm(m_innovation);
    emit updated(m_state, innNorm);
}

/* ---------- 访问器 ---------- */

QVector<double> KalmanFilter2::state() const { return m_state; }

KalmanFilter2::Matrix KalmanFilter2::covariance() const { return m_covariance; }

bool KalmanFilter2::isInitialized() const { return m_initialized; }

QVector<double> KalmanFilter2::innovation() const { return m_innovation; }

/* ---------- 雅可比: 状态转移 ---------- */

KalmanFilter2::Matrix KalmanFilter2::computeStateJacobian(
    const QVector<double>& state) const
{
    Matrix J(m_stateDim, QVector<double>(m_stateDim, 0.0));
    double eps = 1e-6;

    for (int j = 0; j < m_stateDim; ++j) {
        /* 偏移状态向量第j个分量 */
        QVector<double> statePlus = state;
        statePlus[j] += eps;

        QVector<double> f0 = m_stateFunc(state);
        QVector<double> f1 = m_stateFunc(statePlus);

        /* 中心差分: (f(x+eps) - f(x)) / eps */
        for (int i = 0; i < m_stateDim; ++i) {
            J[i][j] = (f1[i] - f0[i]) / eps;
        }
    }

    return J;
}

/* ---------- 雅可比: 观测 ---------- */

KalmanFilter2::Matrix KalmanFilter2::computeObsJacobian(
    const QVector<double>& state) const
{
    Matrix H(m_obsDim, QVector<double>(m_stateDim, 0.0));
    double eps = 1e-6;

    for (int j = 0; j < m_stateDim; ++j) {
        QVector<double> statePlus = state;
        statePlus[j] += eps;

        QVector<double> h0 = m_obsFunc(state);
        QVector<double> h1 = m_obsFunc(statePlus);

        for (int i = 0; i < m_obsDim; ++i) {
            H[i][j] = (h1[i] - h0[i]) / eps;
        }
    }

    return H;
}

/* ---------- 矩阵运算 ---------- */

KalmanFilter2::Matrix KalmanFilter2::matMul(
    const Matrix& A, const Matrix& B) const
{
    int m = A.size();
    if (m == 0) return Matrix();
    int n = B.isEmpty() ? 0 : B[0].size();
    int p = B.size();
    if (p == 0) return Matrix();

    Matrix C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < p; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

KalmanFilter2::Matrix KalmanFilter2::matTranspose(const Matrix& A) const
{
    if (A.isEmpty()) return Matrix();
    int m = A.size();
    int n = A[0].size();
    Matrix T(n, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            T[j][i] = A[i][j];
    return T;
}

KalmanFilter2::Matrix KalmanFilter2::matAdd(
    const Matrix& A, const Matrix& B) const
{
    int m = A.size();
    if (m == 0) return Matrix();
    int n = A[0].size();
    Matrix C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

KalmanFilter2::Matrix KalmanFilter2::matSub(
    const Matrix& A, const Matrix& B) const
{
    int m = A.size();
    if (m == 0) return Matrix();
    int n = A[0].size();
    Matrix C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] - B[i][j];
    return C;
}

KalmanFilter2::Matrix KalmanFilter2::matInverse(const Matrix& A) const
{
    int n = A.size();
    if (n == 0) return Matrix();

    /* 增广矩阵 [A | I] */
    Matrix aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = A[i][j];
        aug[i][n + i] = 1.0;
    }

    /* Gauss-Jordan消元 */
    for (int col = 0; col < n; ++col) {
        /* 选主元 */
        int maxRow = col;
        for (int row = col + 1; row < n; ++row) {
            if (std::abs(aug[row][col]) > std::abs(aug[maxRow][col])) {
                maxRow = row;
            }
        }
        std::swap(aug[col], aug[maxRow]);

        double pivot = aug[col][col];
        if (std::abs(pivot) < 1e-12) continue;

        for (int j = 0; j < 2 * n; ++j) aug[col][j] /= pivot;

        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j) {
                aug[row][j] -= factor * aug[col][j];
            }
        }
    }

    /* 提取逆矩阵 */
    Matrix inv(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            inv[i][j] = aug[i][n + j];
    return inv;
}

KalmanFilter2::Matrix KalmanFilter2::identity(int n) const
{
    Matrix I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

double KalmanFilter2::vecNorm(const QVector<double>& v) const
{
    double sum = 0.0;
    for (double val : v) sum += val * val;
    return std::sqrt(sum);
}

/* ---------- 统计 ---------- */

KalmanFilter2::Stats KalmanFilter2::stats() const { return m_stats; }

void KalmanFilter2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
