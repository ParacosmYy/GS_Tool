/**
 * @file GeneralizedEigen2.cpp
 * @brief 广义特征值求解实现
 */

#include "utils/matrix29/GeneralizedEigen2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

GeneralizedEigen2::GeneralizedEigen2(QObject* parent)
    : QObject(parent)
    , m_maxIterations(300)
    , m_tolerance(1e-10)
    , m_timeSum(0.0)
{
}

void GeneralizedEigen2::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(1, maxIter);
}

void GeneralizedEigen2::setTolerance(double tol)
{
    m_tolerance = qMax(1e-15, tol);
}

GeneralizedEigen2::SolveResult GeneralizedEigen2::solve(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = A.size();
    if (n == 0 || B.size() != n) return result;

    /* QZ分解 */
    SchurResult schur = qzDecompose(A, B);
    result.eigenvalues = schur.eigenvalues;
    result.iterations = 0;
    result.residual = 0.0;

    /* 从Schur形式反代求特征向量 */
    result.eigenvectors.resize(n);
    for (int i = 0; i < n; ++i) {
        QVector<double> v(n, 0.0);
        v[i] = 1.0;
        /* 反代: (S - λ*T)·x = 0 取最小主元对应的向量 */
        if (i > 0) {
            double sr = schur.S[i][i];
            double si = 0.0;
            double tr = schur.T[i][i];
            double ti = 0.0;
            for (int j = i - 1; j >= 0; --j) {
                double num = 0.0;
                for (int k = j + 1; k <= i; ++k) {
                    num += schur.S[j][k] * v[k] - sr * schur.T[j][k] * v[k];
                }
                double denom = schur.S[j][j] - sr * schur.T[j][j] / tr * tr;
                if (qAbs(denom) > m_tolerance)
                    v[j] = -num / denom;
                else
                    v[j] = 0.0;
            }
        }
        /* 正交变换回原空间: x = Z·v */
        QVector<double> eigvec(n, 0.0);
        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                eigvec[r] += schur.Z[r][c] * v[c];
            }
        }
        /* 归一化 */
        double norm = 0.0;
        for (double val : eigvec) norm += val * val;
        norm = qSqrt(norm);
        if (norm > m_tolerance) {
            for (double& val : eigvec) val /= norm;
        }
        result.eigenvectors[i] = eigvec;
    }

    /* 计算残差 */
    double totalResidual = 0.0;
    for (int i = 0; i < result.eigenvalues.size(); ++i) {
        const Eigenvalue& ev = result.eigenvalues[i];
        if (!ev.isFinite || i >= result.eigenvectors.size()) continue;
        double res = computeResidual(A, B, ev, result.eigenvectors[i]);
        totalResidual += res * res;
    }
    result.residual = qSqrt(totalResidual);

    ++m_stats.totalSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveComplete(result.eigenvalues.size(), result.iterations);
    return result;
}

GeneralizedEigen2::SchurResult GeneralizedEigen2::qzDecompose(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    int n = A.size();
    SchurResult result;
    if (n == 0) return result;

    /* 复制输入矩阵 */
    QVector<QVector<double>> S = A, T = B;
    result.Q = identity(n);
    result.Z = identity(n);

    /* Step 1: Hessenberg化简 */
    hessenbergReduce(S, T, result.Q, result.Z);

    /* Step 2: QZ迭代 */
    int iter = 0;
    for (iter = 0; iter < m_maxIterations; ++iter) {
        /* 检查B的对角元素是否接近零( deflate ) */
        for (int i = 0; i < n; ++i) {
            if (qAbs(T[i][i]) < m_tolerance * matrixNorm(T)) {
                T[i][i] = 0.0;
            }
        }

        /* 找活跃子矩阵 [lo, hi] */
        int hi = n - 1;
        while (hi > 0 && qAbs(T[hi][hi]) < m_tolerance) --hi;
        if (hi <= 0) break;

        int lo = hi - 1;
        while (lo > 0 && qAbs(T[lo][lo]) >= m_tolerance) --lo;
        if (qAbs(T[lo][lo]) < m_tolerance) ++lo;

        if (lo >= hi) break;

        /* 对活跃子矩阵执行QZ步 */
        qzStep(S, T, result.Q, result.Z, lo, hi);

        emit iterationStep(iter, S[hi][hi]);
    }

    result.S = S;
    result.T = T;
    result.eigenvalues = extractEigenvalues(S, T);

    m_stats.totalIterations += iter;
    if (m_stats.totalSolves > 0)
        m_stats.avgIterations = static_cast<double>(m_stats.totalIterations)
            / m_stats.totalSolves;

    return result;
}

double GeneralizedEigen2::computeResidual(const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B, const Eigenvalue& lambda,
    const QVector<double>& v) const
{
    int n = v.size();
    /* Av - λ·Bv */
    double residual = 0.0;
    for (int i = 0; i < n; ++i) {
        double avi = 0.0, bvi = 0.0;
        for (int j = 0; j < n; ++j) {
            avi += A[i][j] * v[j];
            bvi += B[i][j] * v[j];
        }
        double diff = avi - (lambda.real * bvi);
        residual += diff * diff;
    }
    return qSqrt(residual);
}

void GeneralizedEigen2::hessenbergReduce(QVector<QVector<double>>& A,
    QVector<QVector<double>>& B,
    QVector<QVector<double>>& Q,
    QVector<QVector<double>>& Z)
{
    int n = A.size();

    /* B的上三角化(Householder) */
    for (int k = n - 1; k >= 1; --k) {
        QVector<double> col(k + 1);
        for (int i = 0; i <= k; ++i) col[i] = B[i][k];
        double beta;
        QVector<double> v = col;
        householder(v, beta, k + 1);
        if (qAbs(beta) < m_tolerance) continue;

        /* B := (I - beta*v*v')*B */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i <= k; ++i) dot += v[i] * B[i][j];
            for (int i = 0; i <= k; ++i) B[i][j] -= beta * v[i] * dot;
        }
        /* A := (I - beta*v*v')*A */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i <= k; ++i) dot += v[i] * A[i][j];
            for (int i = 0; i <= k; ++i) A[i][j] -= beta * v[i] * dot;
        }
        /* Q := Q*(I - beta*v*v') */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j <= k; ++j) dot += Q[i][j] * v[j];
            for (int j = 0; j <= k; ++j) Q[i][j] -= beta * dot * v[j];
        }
    }

    /* A的Hessenberg化简 */
    for (int k = 0; k < n - 2; ++k) {
        QVector<double> col(n - k - 1);
        for (int i = 0; i < n - k - 1; ++i) col[i] = A[k + 1 + i][k];
        double beta;
        QVector<double> v = col;
        householder(v, beta, n - k - 1);
        if (qAbs(beta) < m_tolerance) continue;

        int m = n - k - 1;
        /* A := (I - beta*v*v')*A */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < m; ++i) dot += v[i] * A[k + 1 + i][j];
            for (int i = 0; i < m; ++i) A[k + 1 + i][j] -= beta * v[i] * dot;
        }
        /* A := A*(I - beta*v*v') */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < m; ++j) dot += A[i][k + 1 + j] * v[j];
            for (int j = 0; j < m; ++j) A[i][k + 1 + j] -= beta * dot * v[j];
        }
        /* B := B*(I - beta*v*v') */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < m; ++j) dot += B[i][k + 1 + j] * v[j];
            for (int j = 0; j < m; ++j) B[i][k + 1 + j] -= beta * dot * v[j];
        }
    }
}

void GeneralizedEigen2::qzStep(QVector<QVector<double>>& A,
    QVector<QVector<double>>& B,
    QVector<QVector<double>>& Q,
    QVector<QVector<double>>& Z, int lo, int hi)
{
    /* 简化QZ步: Givens旋转消元 */
    for (int k = lo; k < hi; ++k) {
        /* 消B[k+1][k]: Givens旋转 */
        double a = B[k][k];
        double b = B[k + 1][k];
        if (qAbs(b) < m_tolerance) continue;
        double r = qSqrt(a * a + b * b);
        double c = a / r;
        double s = b / r;

        /* 右旋转 Z */
        for (int i = 0; i < A.size(); ++i) {
            double t1 = A[i][k], t2 = A[i][k + 1];
            A[i][k] = c * t1 + s * t2;
            A[i][k + 1] = -s * t1 + c * t2;
            double u1 = B[i][k], u2 = B[i][k + 1];
            B[i][k] = c * u1 + s * u2;
            B[i][k + 1] = -s * u1 + c * u2;
        }
        for (int i = 0; i < A.size(); ++i) {
            double z1 = Z[i][k], z2 = Z[i][k + 1];
            Z[i][k] = c * z1 + s * z2;
            Z[i][k + 1] = -s * z1 + c * z2;
        }

        /* 左旋转 Q: 消A[k+1][k]以下 */
        double pa = A[k][k];
        double pb = A[k + 1][k];
        if (qAbs(pb) > m_tolerance) {
            double pr = qSqrt(pa * pa + pb * pb);
            double pc = pa / pr;
            double ps = pb / pr;
            for (int j = 0; j < A.size(); ++j) {
                double t1 = A[k][j], t2 = A[k + 1][j];
                A[k][j] = pc * t1 + ps * t2;
                A[k + 1][j] = -ps * t1 + pc * t2;
                double u1 = B[k][j], u2 = B[k + 1][j];
                B[k][j] = pc * u1 + ps * u2;
                B[k + 1][j] = -ps * u1 + pc * u2;
            }
            for (int i = 0; i < A.size(); ++i) {
                double q1 = Q[k][i], q2 = Q[k + 1][i];
                Q[k][i] = pc * q1 + ps * q2;
                Q[k + 1][i] = -ps * q1 + pc * q2;
            }
        }
    }
}

void GeneralizedEigen2::householder(QVector<double>& v, double& beta,
    int size) const
{
    Q_UNUSED(size)
    double sigma = 0.0;
    int n = v.size();
    if (n <= 0) { beta = 0.0; return; }
    double x0 = v[0];
    for (int i = 1; i < n; ++i) sigma += v[i] * v[i];

    if (sigma < m_tolerance * m_tolerance) {
        beta = 0.0;
        return;
    }
    double mu = qSqrt(x0 * x0 + sigma);
    double v0 = (x0 <= 0.0) ? x0 - mu : -sigma / (x0 + mu);
    beta = 2.0 * v0 * v0 / (sigma + v0 * v0);
    for (int i = 1; i < n; ++i) v[i] /= v0;
    v[0] = 1.0;
}

QList<GeneralizedEigen2::Eigenvalue> GeneralizedEigen2::extractEigenvalues(
    const QVector<QVector<double>>& S,
    const QVector<QVector<double>>& T) const
{
    int n = S.size();
    QList<Eigenvalue> eigenvalues;
    for (int i = 0; i < n; ++i) {
        Eigenvalue ev;
        if (qAbs(T[i][i]) > m_tolerance) {
            ev.real = S[i][i] / T[i][i];
            ev.isFinite = true;
        } else {
            ev.real = (qAbs(S[i][i]) > m_tolerance)
                ? std::numeric_limits<double>::infinity() : 0.0;
            ev.isFinite = false;
        }
        ev.imag = 0.0;
        eigenvalues.append(ev);
    }
    return eigenvalues;
}

double GeneralizedEigen2::matrixNorm(const QVector<QVector<double>>& M) const
{
    double norm = 0.0;
    for (const auto& row : M) {
        for (double val : row) norm += val * val;
    }
    return qSqrt(norm);
}

QVector<QVector<double>> GeneralizedEigen2::identity(int n) const
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) I[i][i] = 1.0;
    return I;
}

void GeneralizedEigen2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
