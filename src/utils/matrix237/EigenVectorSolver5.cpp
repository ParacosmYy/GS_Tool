/**
 * @file EigenVectorSolver5.cpp
 * @brief EigenVectorSolver5 实现
 *
 * 实现特征向量求解器：Lanczos双正交化与前瞻Lanczos避免崩溃。
 */

#include "utils/matrix237/EigenVectorSolver5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EigenVectorSolver5::EigenVectorSolver5(QObject *parent) : QObject(parent) {}
EigenVectorSolver5::~EigenVectorSolver5() = default;

/* ---- Configuration ---- */

void EigenVectorSolver5::setNumEigenPairs(int k) { m_numPairs = qMax(1, k); }
void EigenVectorSolver5::setMaxIterations(int maxIter) { m_maxIter = qMax(10, maxIter); }

/* ---- Helpers ---- */

QVector<double> EigenVectorSolver5::matVec(const QVector<QVector<double>>& A,
                                             const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

double EigenVectorSolver5::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double EigenVectorSolver5::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Lanczos biorthogonalization with look-ahead ---- */

bool EigenVectorSolver5::lanczosBiortho(const QVector<QVector<double>>& A,
                                          QVector<QVector<double>>& V,
                                          QVector<QVector<double>>& W,
                                          QVector<double>& alpha,
                                          QVector<double>& beta,
                                          int& actualDim)
{
    int n = A.size();
    int maxK = qMin(m_maxIter, n);

    // Initialize with random vectors
    QVector<double> v(n, 0.0), w(n, 0.0);
    for (int i = 0; i < n; ++i) {
        v[i] = qSin(i + 1.0);
        w[i] = qCos(i + 1.0);
    }
    double nv = norm(v);
    double nw = norm(w);
    for (int i = 0; i < n; ++i) { v[i] /= nv; w[i] /= nw; }

    V.resize(maxK + 1);
    W.resize(maxK + 1);
    alpha.resize(maxK);
    beta.resize(maxK);

    V[0] = v;
    W[0] = w;

    actualDim = 0;
    for (int j = 0; j < maxK; ++j) {
        QVector<double> Av = matVec(A, V[j]);
        QVector<double> Atw = matVec(A, W[j]);  // A^T * w (assuming A is real, A^T needed)
        // For symmetric: A^T = A, use same
        // For general: transpose the matrix access
        int n2 = A.size();
        QVector<double> AtW(n2, 0.0);
        for (int i = 0; i < n2; ++i)
            for (int k = 0; k < n2; ++k)
                AtW[i] += A[k][i] * W[j][k];  // A^T * w_j

        // alpha_j = w_j^T * A * v_j
        alpha[j] = dot(W[j], Av);

        // v_{j+1} = A*v_j - alpha_j*v_j - beta_{j-1}*v_{j-1}
        QVector<double> vNext(n, 0.0);
        for (int i = 0; i < n; ++i) {
            vNext[i] = Av[i] - alpha[j] * V[j][i];
            if (j > 0) vNext[i] -= beta[j - 1] * V[j - 1][i];
        }

        // w_{j+1} = A^T*w_j - alpha_j*w_j - beta_{j-1}*w_{j-1}
        QVector<double> wNext(n, 0.0);
        for (int i = 0; i < n; ++i) {
            wNext[i] = AtW[i] - alpha[j] * W[j][i];
            if (j > 0) wNext[i] -= beta[j - 1] * W[j - 1][i];
        }

        // Check for breakdown (near-zero beta)
        double bVal = dot(wNext, vNext);
        if (qAbs(bVal) < 1e-15) {
            // Look-ahead Lanczos: attempt to skip this breakdown
            m_stats.numLookahead++;
            // Use a perturbed vector to continue
            for (int i = 0; i < n; ++i)
                vNext[i] += 1e-8 * qSin(i * 3.7 + j * 2.1);
            for (int i = 0; i < n; ++i)
                wNext[i] += 1e-8 * qCos(i * 2.3 + j * 1.7);
            bVal = dot(wNext, vNext);
            if (qAbs(bVal) < 1e-20) {
                m_stats.numBreakdowns++;
                actualDim = j + 1;
                return j > 0;  // partial result
            }
        }

        beta[j] = qSqrt(qAbs(bVal));
        if (beta[j] < 1e-15) beta[j] = 1e-15;

        // Biorthogonalize: normalize
        double signB = (bVal >= 0) ? 1.0 : -1.0;
        for (int i = 0; i < n; ++i) {
            V[j + 1][i] = vNext[i] / beta[j];
            W[j + 1][i] = wNext[i] / (signB * beta[j]);
        }

        actualDim = j + 1;
    }
    return true;
}

/* ---- Tridiagonal eigen (implicit QR) ---- */

void EigenVectorSolver5::tridiagEigen(const QVector<double>& alpha,
                                        const QVector<double>& beta,
                                        QVector<double>& eigenvalues,
                                        QVector<QVector<double>>& eigenvectors) const
{
    int n = alpha.size();
    if (n == 0) return;

    // Copy to working arrays
    QVector<double> d = alpha;
    QVector<double> e(n, 0.0);
    for (int i = 0; i < n - 1; ++i) e[i] = beta[i];

    // Initialize eigenvectors to identity
    eigenvectors.resize(n);
    for (int i = 0; i < n; ++i) {
        eigenvectors[i].resize(n, 0.0);
        eigenvectors[i][i] = 1.0;
    }

    // Implicit QR iteration
    for (int iter = 0; iter < 100 * n; ++iter) {
        // Check convergence of off-diagonal
        double offDiag = 0.0;
        for (int i = 0; i < n - 1; ++i) offDiag += qAbs(e[i]);
        if (offDiag < 1e-14) break;

        // Wilkinson shift
        double dd = (d[n - 2] - d[n - 1]) * 0.5;
        double ee = e[n - 2] * e[n - 2];
        double mu = d[n - 1] - ee / (dd + qSign(dd) * qSqrt(dd * dd + ee));

        // Chase bulge
        double x = d[0] - mu;
        double z = e[0];
        for (int k = 0; k < n - 1; ++k) {
            double c, s;
            if (qAbs(z) < 1e-30) { c = 1.0; s = 0.0; }
            else {
                double r = qSqrt(x * x + z * z);
                c = x / r;
                s = z / r;
            }

            // Apply rotation to tridiagonal
            double w = c * x + s * z;
            double dq = d[k];
            double dk1 = d[k + 1];
            double ek = e[k];

            d[k] = c * c * dq + 2.0 * c * s * ek + s * s * dk1;
            d[k + 1] = s * s * dq - 2.0 * c * s * ek + c * c * dk1;
            e[k] = c * s * (dk1 - dq) + (c * c - s * s) * ek;

            if (k > 0) e[k - 1] = w;

            x = e[k];
            if (k < n - 2) {
                z = -s * e[k + 1];
                e[k + 1] = c * e[k + 1];
            }

            // Update eigenvectors
            for (int i = 0; i < n; ++i) {
                double v1 = eigenvectors[i][k];
                double v2 = eigenvectors[i][k + 1];
                eigenvectors[i][k] = c * v1 + s * v2;
                eigenvectors[i][k + 1] = -s * v1 + c * v2;
            }
        }
    }

    eigenvalues = d;
}

/* ---- Solve ---- */

QVector<EigenVectorSolver5::EigenPair> EigenVectorSolver5::solve(
    const QVector<QVector<double>>& matrix, double tol)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return {};

    m_stats.matrixSize = n;
    m_tol = tol;

    // Run Lanczos biorthogonalization
    QVector<QVector<double>> V, W;
    QVector<double> alpha, beta;
    int krylovDim = 0;

    lanczosBiortho(matrix, V, W, alpha, beta, krylovDim);
    m_stats.krylovDim = krylovDim;

    // Solve tridiagonal eigenvalue problem
    QVector<double> evals;
    QVector<QVector<double>> evecsKrylov;
    tridiagEigen(alpha, beta, evals, evecsKrylov);

    // Sort by absolute eigenvalue (largest first)
    QVector<int> idx(evals.size());
    for (int i = 0; i < idx.size(); ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return qAbs(evals[a]) > qAbs(evals[b]);
    });

    // Recover Ritz vectors and form eigenpairs
    int k = qMin(m_numPairs, krylovDim);
    m_eigenPairs.resize(k);
    m_residuals.resize(k);

    for (int p = 0; p < k; ++p) {
        int i = idx[p];
        m_eigenPairs[p].eigenvalue = evals[i];

        // Ritz vector = V * eigenvector_tridiag
        int nn = n;
        QVector<double> ritzVec(nn, 0.0);
        for (int j = 0; j < krylovDim; ++j)
            for (int l = 0; l < nn; ++l)
                ritzVec[l] += V[j][l] * evecsKrylov[i][j];

        // Normalize
        double nrm = norm(ritzVec);
        if (nrm > 1e-15)
            for (int l = 0; l < nn; ++l) ritzVec[l] /= nrm;

        m_eigenPairs[p].eigenvector = ritzVec;

        // Compute residual ||A*x - lambda*x||
        QVector<double> Ax = matVec(matrix, ritzVec);
        double res = 0.0;
        for (int l = 0; l < nn; ++l)
            res += (Ax[l] - evals[i] * ritzVec[l]) * (Ax[l] - evals[i] * ritzVec[l]);
        m_residuals[p] = qSqrt(res);
    }

    m_stats.iterationsUsed = krylovDim;
    m_stats.residual = m_residuals.isEmpty() ? 0.0 : m_residuals[0];
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(k, timer.elapsed());
    return m_eigenPairs;
}

/* ---- Residuals ---- */

QVector<double> EigenVectorSolver5::residuals() const { return m_residuals; }

/* ---- Reset ---- */

void EigenVectorSolver5::resetStatistics()
{
    m_eigenPairs.clear();
    m_residuals.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
