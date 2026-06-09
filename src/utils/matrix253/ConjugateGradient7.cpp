/**
 * @file ConjugateGradient7.cpp
 * @brief ConjugateGradient7 实现
 *
 * 实现共轭梯度法：降阶Lanczos与增广Krylov子空间多特征对收敛。
 */

#include "utils/matrix253/ConjugateGradient7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

ConjugateGradient7::ConjugateGradient7(QObject *parent) : QObject(parent) {}
ConjugateGradient7::~ConjugateGradient7() = default;

/* ---- Configuration ---- */

void ConjugateGradient7::setTolerance(double tol, int maxIter)
{
    m_tolerance = qMax(1e-15, tol);
    m_maxIter = qMax(1, maxIter);
}

/* ---- Matrix-vector multiplication ---- */

QVector<double> ConjugateGradient7::matVec(const QVector<QVector<double>>& A,
                                            const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int dim = qMin(A[i].size(), x.size());
        for (int j = 0; j < dim; ++j)
            y[i] += A[i][j] * x[j];
    }
    return y;
}

/* ---- Dot product ---- */

double ConjugateGradient7::dot(const QVector<double>& a,
                                const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double ConjugateGradient7::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Lanczos tridiagonalization ---- */

void ConjugateGradient7::lanczos(const QVector<QVector<double>>& A, int steps,
                                  QVector<double>& alpha, QVector<double>& beta,
                                  QVector<QVector<double>>& Q) const
{
    int n = A.size();
    Q.resize(steps);
    alpha.resize(steps);
    beta.resize(steps, 0.0);

    // Initial random vector
    QVector<double> q(n, 0.0);
    double nrm = 0.0;
    for (int i = 0; i < n; ++i) {
        q[i] = 1.0 + static_cast<double>(qrand() % 1000) / 1000.0;
        nrm += q[i] * q[i];
    }
    nrm = qSqrt(nrm);
    for (auto& v : q) v /= nrm;

    Q[0] = q;
    QVector<double> prevQ(n, 0.0);

    for (int j = 0; j < steps; ++j) {
        QVector<double> w = matVec(A, Q[j]);
        alpha[j] = dot(w, Q[j]);

        // w = w - alpha[j]*Q[j] - beta[j]*Q[j-1]
        for (int i = 0; i < n; ++i) {
            w[i] -= alpha[j] * Q[j][i];
            if (j > 0) w[i] -= beta[j] * Q[j - 1][i];
        }

        // Reorthogonalize
        for (int k = 0; k <= j; ++k) {
            double d = dot(w, Q[k]);
            for (int i = 0; i < n; ++i) w[i] -= d * Q[k][i];
        }

        if (j < steps - 1) {
            beta[j + 1] = norm(w);
            if (beta[j + 1] < 1e-15) { steps = j + 1; break; }
            Q[j + 1].resize(n);
            for (int i = 0; i < n; ++i)
                Q[j + 1][i] = w[i] / beta[j + 1];
        }
    }
}

/* ---- QR algorithm for tridiagonal eigenvalue problem ---- */

void ConjugateGradient7::tridiagQR(QVector<double>& diag,
                                    QVector<double>& subdiag,
                                    QVector<QVector<double>>& eigvecs) const
{
    int n = diag.size();
    eigvecs.resize(n);
    for (int i = 0; i < n; ++i) {
        eigvecs[i].resize(n, 0.0);
        eigvecs[i][i] = 1.0;
    }

    for (int iter = 0; iter < 100 * n; ++iter) {
        // Check convergence
        double offDiag = 0.0;
        for (int i = 0; i < n - 1; ++i) offDiag += qAbs(subdiag[i]);
        if (offDiag < 1e-12) break;

        // Wilkinson shift
        double d = (diag[n - 2] - diag[n - 1]) / 2.0;
        double mu = diag[n - 1] - subdiag[n - 1] * subdiag[n - 1]
            / (d + (d >= 0 ? 1.0 : -1.0) * qSqrt(d * d + subdiag[n - 1] * subdiag[n - 1]));

        // QR step with shift
        double x = diag[0] - mu;
        double z = subdiag[0];
        for (int i = 0; i < n - 1; ++i) {
            double r = qSqrt(x * x + z * z);
            if (r < 1e-15) r = 1e-15;
            double c = x / r, s = z / r;

            // Apply Givens rotation
            if (i > 0) subdiag[i - 1] = r;
            double w = c * diag[i] + s * subdiag[i];
            double y = -s * diag[i] + c * subdiag[i];
            diag[i] = w;
            subdiag[i] = y;
            x = subdiag[i];
            w = c * subdiag[i] + s * diag[i + 1];
            y = -s * subdiag[i] + c * diag[i + 1];
            diag[i + 1] = w;
            if (i < n - 2) { z = s * subdiag[i + 1]; subdiag[i + 1] *= c; }

            // Update eigenvectors
            for (int k = 0; k < n; ++k) {
                double e1 = eigvecs[k][i], e2 = eigvecs[k][i + 1];
                eigvecs[k][i] = c * e1 + s * e2;
                eigvecs[k][i + 1] = -s * e1 + c * e2;
            }
        }
    }
}

/* ---- Solve Ax = b ---- */

ConjugateGradient7::SolveResult ConjugateGradient7::solve(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();
    if (n == 0) return result;

    result.solution.resize(n, 0.0);
    QVector<double> r = b; // r = b - A*x, x=0
    QVector<double> p = r;
    double rsOld = dot(r, r);

    for (int i = 0; i < m_maxIter && i < n; ++i) {
        QVector<double> Ap = matVec(A, p);
        double pAp = dot(p, Ap);
        if (qAbs(pAp) < 1e-20) break;
        double alpha = rsOld / pAp;

        for (int j = 0; j < n; ++j)
            result.solution[j] += alpha * p[j];

        for (int j = 0; j < n; ++j)
            r[j] -= alpha * Ap[j];

        double rsNew = dot(r, r);
        result.residual = qSqrt(rsNew);
        result.iterations = i + 1;

        if (result.residual < m_tolerance) {
            result.converged = true;
            break;
        }

        double beta = rsNew / qMax(rsOld, 1e-20);
        for (int j = 0; j < n; ++j)
            p[j] = r[j] + beta * p[j];

        rsOld = rsNew;
    }

    m_stats.matrixSize = n;
    m_stats.numIterations = result.iterations;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(result.iterations, result.residual, timer.elapsed());
    return result;
}

/* ---- Compute eigenpairs via deflated Lanczos ---- */

QVector<ConjugateGradient7::EigenPair> ConjugateGradient7::eigenpairs(
    const QVector<QVector<double>>& A, int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    k = qMin(k, n);
    int steps = qMin(qMax(2 * k + 10, 30), n);

    QVector<double> alpha, beta;
    QVector<QVector<double>> Q;
    lanczos(A, steps, alpha, beta, Q);

    // Solve tridiagonal eigenvalue problem
    int m = alpha.size();
    QVector<double> subdiag(m, 0.0);
    for (int i = 0; i < m - 1; ++i) subdiag[i] = beta[i + 1];

    QVector<QVector<double>> eigvecsT;
    tridiagQR(alpha, subdiag, eigvecsT);

    // Sort by eigenvalue descending
    QVector<int> order(m);
    for (int i = 0; i < m; ++i) order[i] = i;
    for (int i = 0; i < m - 1; ++i)
        for (int j = i + 1; j < m; ++j)
            if (alpha[order[j]] > alpha[order[i]])
                qSwap(order[i], order[j]);

    QVector<EigenPair> result;
    for (int idx = 0; idx < k && idx < m; ++idx) {
        EigenPair ep;
        ep.eigenvalue = alpha[order[idx]];
        ep.eigenvector.resize(n, 0.0);
        // Recover eigenvector from Lanczos vectors
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m && j < Q.size(); ++j)
                ep.eigenvector[i] += Q[j][i] * eigvecsT[order[idx]][j];
        // Normalize
        double nrm = norm(ep.eigenvector);
        if (nrm > 1e-15)
            for (auto& v : ep.eigenvector) v /= nrm;
        result.append(ep);
    }

    m_stats.numEigenpairs = result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Reset ---- */

void ConjugateGradient7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
