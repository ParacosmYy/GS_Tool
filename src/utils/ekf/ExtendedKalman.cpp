/**
 * @file ExtendedKalman.cpp
 * @brief 扩展卡尔曼滤波器实现 — 非线性系统Jacobian
 */

#include "utils/ekf/ExtendedKalman.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ExtendedKalman::ExtendedKalman(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 初始化EKF
 *  @param state 初始状态
 *  @param covariance 初始协方差矩阵 */
void ExtendedKalman::initialize(const QVector<double>& state,
                                const Matrix& covariance)
{
    m_state = state;
    m_P = covariance;
}

/** @brief 预测步骤
 *  @param stateFunc 非线性状态转移函数
 *  @param jacobianF 状态转移Jacobian
 *  @param processNoise 过程噪声协方差Q */
void ExtendedKalman::predict(const VecFunc& stateFunc,
                             const JacobianFunc& jacobianF,
                             const Matrix& processNoise)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_state.size();

    /* 非线性状态转移: x' = f(x) */
    m_state = stateFunc(m_state);

    /* 线性化: F = df/dx */
    Matrix F = jacobianF(m_state);

    /* 协方差预测: P' = F * P * F^T + Q */
    Matrix Ft = matTranspose(F);
    Matrix FPFt = matMul(matMul(F, m_P), Ft);
    m_P = matAdd(FPFt, processNoise);

    /* 保证对称性 */
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double avg = (m_P[i][j] + m_P[j][i]) * 0.5;
            m_P[i][j] = avg;
            m_P[j][i] = avg;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalPredictions;
    m_timeSum += elapsed;
    double total = static_cast<double>(m_stats.totalPredictions + m_stats.totalUpdates);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit predictionCompleted(n);
}

/** @brief 更新步骤
 *  @param obs 观测向量
 *  @param obsFunc 非线性观测函数
 *  @param jacobianH 观测Jacobian
 *  @param measNoise 测量噪声协方差R */
void ExtendedKalman::update(const QVector<double>& obs,
                            const VecFunc& obsFunc,
                            const JacobianFunc& jacobianH,
                            const Matrix& measNoise)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_state.size();
    int m = obs.size();

    /* 观测预测: z_hat = h(x) */
    QVector<double> zHat = obsFunc(m_state);

    /* 新息: y = z - z_hat */
    QVector<double> innovation = vecSub(obs, zHat);

    /* 线性化: H = dh/dx */
    Matrix H = jacobianH(m_state);
    Matrix Ht = matTranspose(H);

    /* 新息协方差: S = H * P * H^T + R */
    Matrix HPHt = matMul(matMul(H, m_P), Ht);
    Matrix S = matAdd(HPHt, measNoise);

    /* 卡尔曼增益: K = P * H^T * S^{-1} */
    Matrix Sinv = matInverse(S);
    Matrix PHt = matMul(m_P, Ht);

    /* K: n x m */
    Matrix K(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            for (int l = 0; l < static_cast<int>(PHt[i].size()); ++l) {
                K[i][j] += PHt[i][l] * Sinv[l][j];
            }
        }
    }

    /* 状态更新: x = x + K * y */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            m_state[i] += K[i][j] * innovation[j];
        }
    }

    /* 协方差更新: P = (I - K * H) * P */
    Matrix KH = matMul(K, H);
    Matrix I_KH = identity(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            I_KH[i][j] -= KH[i][j];
    m_P = matMul(I_KH, m_P);

    double innoNorm = 0.0;
    for (double v : innovation) innoNorm += v * v;
    innoNorm = qSqrt(innoNorm);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUpdates;
    m_timeSum += elapsed;
    double total = static_cast<double>(m_stats.totalPredictions + m_stats.totalUpdates);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit updateCompleted(n, innoNorm);
}

/** @brief 重置统计 */
void ExtendedKalman::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 矩阵乘法 */
ExtendedKalman::Matrix ExtendedKalman::matMul(const Matrix& A, const Matrix& B)
{
    int rows = A.size();
    int cols = (B.size() > 0) ? B[0].size() : 0;
    int inner = (A.size() > 0) ? A[0].size() : 0;
    Matrix C(rows, QVector<double>(cols, 0.0));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            for (int k = 0; k < inner; ++k)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

/** @brief 矩阵转置 */
ExtendedKalman::Matrix ExtendedKalman::matTranspose(const Matrix& M)
{
    if (M.isEmpty()) return Matrix{};
    int rows = M.size(), cols = M[0].size();
    Matrix T(cols, QVector<double>(rows));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            T[j][i] = M[i][j];
    return T;
}

/** @brief 矩阵加法 */
ExtendedKalman::Matrix ExtendedKalman::matAdd(const Matrix& A, const Matrix& B)
{
    int rows = qMin(A.size(), B.size());
    Matrix C(rows);
    for (int i = 0; i < rows; ++i) {
        int cols = qMin(A[i].size(), B[i].size());
        C[i].resize(cols);
        for (int j = 0; j < cols; ++j)
            C[i][j] = A[i][j] + B[i][j];
    }
    return C;
}

/** @brief 矩阵求逆(Gauss-Jordan) */
ExtendedKalman::Matrix ExtendedKalman::matInverse(const Matrix& M)
{
    int n = M.size();
    if (n == 0) return Matrix{};

    Matrix aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = M[i][j];
        aug[i][n + i] = 1.0;
    }

    for (int col = 0; col < n; ++col) {
        int maxRow = col;
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(aug[row][col]) > qAbs(aug[maxRow][col])) maxRow = row;
        }
        std::swap(aug[col], aug[maxRow]);
        double pivot = aug[col][col];
        if (qAbs(pivot) < 1e-15) continue;
        for (int j = 0; j < 2 * n; ++j) aug[col][j] /= pivot;
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j)
                aug[row][j] -= factor * aug[col][j];
        }
    }

    Matrix result(n, QVector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            result[i][j] = aug[i][n + j];
    return result;
}

/** @brief 向量减法 */
QVector<double> ExtendedKalman::vecSub(const QVector<double>& a,
                                       const QVector<double>& b)
{
    int n = qMin(a.size(), b.size());
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = a[i] - b[i];
    return r;
}

/** @brief 单位矩阵 */
ExtendedKalman::Matrix ExtendedKalman::identity(int n)
{
    Matrix I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}
