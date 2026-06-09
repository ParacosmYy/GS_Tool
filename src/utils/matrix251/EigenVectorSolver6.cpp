/**
 * @file EigenVectorSolver6.cpp
 * @brief EigenVectorSolver6 实现
 *
 * 实现特征向量求解器：子空间迭代与Rayleigh-Ritz投影的主特征对提取。
 */

#include "utils/matrix251/EigenVectorSolver6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

EigenVectorSolver6::EigenVectorSolver6(QObject *parent) : QObject(parent) {}
EigenVectorSolver6::~EigenVectorSolver6() = default;

/* ---- Configuration ---- */

void EigenVectorSolver6::setNumEigenpairs(int k) { m_numPairs = qMax(1, k); }
void EigenVectorSolver6::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }
void EigenVectorSolver6::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void EigenVectorSolver6::setSubspaceDimension(int dim)
{
    m_subDim = qMax(m_numPairs, dim);
}

/* ---- Matrix-vector multiply ---- */

QVector<double> EigenVectorSolver6::matVec(
    const QVector<QVector<double>>& A, const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int cols = qMin(A[i].size(), x.size());
        for (int j = 0; j < cols; ++j)
            y[i] += A[i][j] * x[j];
    }
    return y;
}

/* ---- Dot product ---- */

double EigenVectorSolver6::dot(const QVector<double>& a,
                                const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double EigenVectorSolver6::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- Orthonormalize via modified Gram-Schmidt ---- */

void EigenVectorSolver6::orthonormalize(QVector<QVector<double>>& basis) const
{
    int m = basis.size();
    if (m == 0) return;
    int n = basis[0].size();

    for (int j = 0; j < m; ++j) {
        // Subtract projections onto previous vectors
        for (int i = 0; i < j; ++i) {
            double d = dot(basis[j], basis[i]);
            for (int k = 0; k < n; ++k)
                basis[j][k] -= d * basis[i][k];
        }
        // Normalize
        double nrm = norm(basis[j]);
        if (nrm < 1e-14) {
            // Re-randomize if degenerate
            for (int k = 0; k < n; ++k)
                basis[j][k] = static_cast<double>(std::rand()) / RAND_MAX;
            for (int i = 0; i < j; ++i) {
                double d = dot(basis[j], basis[i]);
                for (int k = 0; k < n; ++k)
                    basis[j][k] -= d * basis[i][k];
            }
            nrm = norm(basis[j]);
        }
        double inv = 1.0 / nrm;
        for (int k = 0; k < n; ++k)
            basis[j][k] *= inv;
    }
}

/* ---- Rayleigh-Ritz projection ---- */

QVector<EigenVectorSolver6::EigenPair> EigenVectorSolver6::rayleighRitz(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& Q) const
{
    int m = Q.size();
    int n = Q[0].size();

    // Form projected matrix H = Q^T A Q (m x m)
    QVector<QVector<double>> H(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) {
        auto Aqi = matVec(A, Q[i]);
        for (int j = i; j < m; ++j) {
            double h = dot(Aqi, Q[j]);
            H[i][j] = h;
            H[j][i] = h;
        }
    }

    // Solve small eigenproblem via Jacobi iteration
    // Eigenvectors of H stored as columns
    QVector<QVector<double>> V(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) V[i][i] = 1.0;
    QVector<double> eigenvalues(m, 0.0);

    // Jacobi eigenvalue algorithm for symmetric H
    for (int iter = 0; iter < 100; ++iter) {
        double offDiag = 0.0;
        int pi = 0, pj = 1;
        for (int i = 0; i < m; ++i)
            for (int j = i + 1; j < m; ++j) {
                double a = qAbs(H[i][j]);
                if (a > offDiag) { offDiag = a; pi = i; pj = j; }
            }
        if (offDiag < 1e-14) break;

        // Givens rotation to zero H[pi][pj]
        double app = H[pi][pi], aqq = H[pj][pj], apq = H[pi][pj];
        double theta = (aqq - app) / (2.0 * apq);
        double t = (theta >= 0 ? 1.0 : -1.0) /
                   (qAbs(theta) + qSqrt(1.0 + theta * theta));
        double c = 1.0 / qSqrt(1.0 + t * t);
        double s = t * c;

        // Update H
        for (int k = 0; k < m; ++k) {
            if (k == pi || k == pj) continue;
            double hki = H[k][pi], hkj = H[k][pj];
            H[pi][k] = H[k][pi] = c * hki - s * hkj;
            H[pj][k] = H[k][pj] = s * hki + c * hkj;
        }
        double newPP = c * c * app - 2 * s * c * apq + s * s * aqq;
        double newQQ = s * s * app + 2 * s * c * apq + c * c * aqq;
        H[pi][pi] = newPP; H[pj][pj] = newQQ;
        H[pi][pj] = H[pj][pi] = 0.0;

        // Update eigenvectors
        for (int k = 0; k < m; ++k) {
            double vi = V[k][pi], vj = V[k][pj];
            V[k][pi] = c * vi - s * vj;
            V[k][pj] = s * vi + c * vj;
        }
    }

    for (int i = 0; i < m; ++i) eigenvalues[i] = H[i][i];

    // Sort by descending eigenvalue
    QVector<int> order(m);
    for (int i = 0; i < m; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return eigenvalues[a] > eigenvalues[b];
    });

    // Map Ritz vectors back to original space
    QVector<EigenPair> pairs;
    for (int k = 0; k < qMin(m, m_numPairs); ++k) {
        EigenPair pair;
        pair.eigenvalue = eigenvalues[order[k]];
        pair.eigenvector.resize(n);
        for (int j = 0; j < n; ++j)
            pair.eigenvector[j] = 0.0;
        for (int j = 0; j < m; ++j) {
            double coeff = V[j][order[k]];
            for (int i = 0; i < n; ++i)
                pair.eigenvector[i] += coeff * Q[j][i];
        }
        pairs.append(pair);
    }
    return pairs;
}

/* ---- Main solver ---- */

QVector<EigenVectorSolver6::EigenPair> EigenVectorSolver6::solve(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return {};

    int subDim = qMax(m_numPairs, qMin(m_subDim, n));

    // Initialize random orthonormal basis
    QVector<QVector<double>> Q(subDim, QVector<double>(n));
    for (int j = 0; j < subDim; ++j)
        for (int i = 0; i < n; ++i)
            Q[j][i] = static_cast<double>(std::rand()) / RAND_MAX;
    orthonormalize(Q);

    int iter = 0;
    double residual = 1e18;

    for (; iter < m_maxIter; ++iter) {
        // Power iteration: Q = A * Q
        QVector<QVector<double>> AQ(subDim);
        for (int j = 0; j < subDim; ++j)
            AQ[j] = matVec(matrix, Q[j]);

        // Orthonormalize
        orthonormalize(AQ);
        Q = AQ;

        // Check convergence via Rayleigh quotient residual
        auto ritz = rayleighRitz(matrix, Q);
        if (!ritz.isEmpty()) {
            double maxRes = 0.0;
            for (const auto& p : ritz) {
                auto Ax = matVec(matrix, p.eigenvector);
                int pn = p.eigenvector.size();
                for (int i = 0; i < pn; ++i)
                    Ax[i] -= p.eigenvalue * p.eigenvector[i];
                maxRes = qMax(maxRes, norm(Ax));
            }
            residual = maxRes;
            if (residual < m_tol) break;
        }
    }

    // Final Ritz extraction
    auto result = rayleighRitz(matrix, Q);

    m_stats.matrixSize = n;
    m_stats.numEigenpairs = result.size();
    m_stats.totalIterations = iter;
    m_stats.residual = residual;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solvingCompleted(result.size(), iter, residual, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void EigenVectorSolver6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
