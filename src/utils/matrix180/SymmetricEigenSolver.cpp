/**
 * @file SymmetricEigenSolver.cpp
 * @brief SymmetricEigenSolver 实现
 *
 * 实现对称特征值求解：Householder三对角化、隐式QR迭代(Wilkinson移位)。
 */

#include "utils/matrix180/SymmetricEigenSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SymmetricEigenSolver::SymmetricEigenSolver(QObject *parent)
    : QObject(parent)
{
}

SymmetricEigenSolver::~SymmetricEigenSolver() = default;

/* ---- Configuration ---- */

void SymmetricEigenSolver::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }
void SymmetricEigenSolver::setMaxIterations(int iter) { m_maxIterations = qMax(10, iter); }

/* ---- Householder tridiagonalization ---- */

void SymmetricEigenSolver::tridiagonalize(QVector<QVector<double>>& mat,
                                           QVector<QVector<double>>& Q)
{
    int n = mat.size();

    /* Initialize Q as identity */
    Q.resize(n);
    for (int i = 0; i < n; ++i) {
        Q[i].resize(n, 0.0);
        Q[i][i] = 1.0;
    }

    for (int k = 0; k < n - 2; ++k) {
        /* Build Householder vector from column k below diagonal */
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i)
            sigma += mat[i][k] * mat[i][k];

        double alpha = mat[k + 1][k];
        double normX = qSqrt(alpha * alpha + sigma);

        if (normX < 1e-15) continue;

        double sign = (alpha >= 0) ? 1.0 : -1.0;
        double v1 = alpha + sign * normX;
        double v1sq = v1 * v1;
        double beta = 2.0 * v1sq / (sigma + v1sq);

        /* Householder vector v: v[0] = 1, v[i] = mat[k+1+i][k] / v1 */
        QVector<double> v(n - k - 1, 0.0);
        v[0] = 1.0;
        for (int i = 1; i < n - k - 1; ++i)
            v[i] = mat[k + 1 + i][k] / v1;

        /* Apply: mat = (I - beta*v*v^T) * mat * (I - beta*v*v^T) */
        /* P = I - beta * v * v^T */
        /* Compute w = beta * mat * v (restricted rows/cols k+1..n-1) */
        int m = n - k - 1;
        QVector<double> w(m, 0.0);
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < m; ++j)
                w[i] += mat[k + 1 + i][k + 1 + j] * v[j];

        /* Compute p = w - (beta/2)*(v^T*w)*v */
        double vTw = 0.0;
        for (int i = 0; i < m; ++i) vTw += v[i] * w[i];

        for (int i = 0; i < m; ++i)
            w[i] = beta * (w[i] - 0.5 * beta * vTw * v[i]);

        /* Update matrix: mat -= v*p^T + p*v^T */
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < m; ++j)
                mat[k + 1 + i][k + 1 + j] -= v[i] * w[j] + w[i] * v[j];

        /* Zero out below subdiagonal */
        for (int i = k + 2; i < n; ++i)
            mat[i][k] = mat[k][i] = 0.0;

        /* Accumulate transformation into Q */
        for (int row = 0; row < n; ++row) {
            double dot = 0.0;
            for (int j = 0; j < m; ++j)
                dot += Q[row][k + 1 + j] * v[j];
            for (int j = 0; j < m; ++j)
                Q[row][k + 1 + j] -= beta * dot * v[j];
        }
    }
}

/* ---- Wilkinson shift ---- */

double SymmetricEigenSolver::wilkinsonShift(double d1, double d2, double e)
{
    double delta = (d1 - d2) / 2.0;
    double sign = (delta >= 0) ? 1.0 : -1.0;
    return d2 - e * e / (delta + sign * qSqrt(delta * delta + e * e));
}

/* ---- Implicit QR step ---- */

void SymmetricEigenSolver::implicitQRStep(QVector<double>& diag,
                                           QVector<double>& sub,
                                           QVector<QVector<double>>& Q,
                                           int lo, int hi)
{
    int n = diag.size();
    double d1 = diag[hi - 1];
    double d2 = diag[hi];
    double e = sub[hi - 1];
    double shift = wilkinsonShift(d1, d2, e);

    /* Implicit QR with Givens rotations */
    double x = diag[lo] - shift;
    double z = sub[lo];

    for (int k = lo; k < hi; ++k) {
        /* Compute Givens rotation to zero out z */
        double r = qSqrt(x * x + z * z);
        double c = (r < 1e-15) ? 1.0 : x / r;
        double s = (r < 1e-15) ? 0.0 : -z / r;

        /* Apply Givens rotation to tridiagonal matrix */
        if (k > lo) sub[k - 1] = r;

        double dK = diag[k];
        double dK1 = diag[k + 1];
        double eK = sub[k];

        diag[k] = c * c * dK - 2.0 * c * s * eK + s * s * dK1;
        diag[k + 1] = s * s * dK + 2.0 * c * s * eK + c * c * dK1;
        sub[k] = c * s * (dK - dK1) + (c * c - s * s) * eK;

        if (k + 1 < hi) {
            x = sub[k];
            z = -s * sub[k + 1];
            sub[k + 1] *= c;
        }

        /* Accumulate eigenvectors */
        for (int i = 0; i < n; ++i) {
            double qIK = Q[i][k];
            double qIK1 = Q[i][k + 1];
            Q[i][k] = c * qIK - s * qIK1;
            Q[i][k + 1] = s * qIK + c * qIK1;
        }
    }
}

/* ---- Solve (eigenvalues only) ---- */

QVector<double> SymmetricEigenSolver::solve(
    const QVector<QVector<double>>& matrix)
{
    auto result = solveWithVectors(matrix);
    return result.first;
}

/* ---- Solve with eigenvectors ---- */

QPair<QVector<double>, QVector<QVector<double>>>
SymmetricEigenSolver::solveWithVectors(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return {{}, {}};

    /* Copy matrix */
    QVector<QVector<double>> mat = matrix;

    /* Step 1: Tridiagonalize */
    QVector<QVector<double>> Q;
    tridiagonalize(mat, Q);

    /* Extract diagonal and subdiagonal */
    QVector<double> diag(n);
    QVector<double> sub(n - 1);
    for (int i = 0; i < n; ++i)
        diag[i] = mat[i][i];
    for (int i = 0; i < n - 1; ++i)
        sub[i] = mat[i + 1][i];

    /* Step 2: Implicit QR iteration */
    int totalIter = 0;
    int hi = n - 1;

    while (hi > 0 && totalIter < m_maxIterations * n) {
        /* Find smallest unreduced subdiagonal element */
        int lo = hi;
        while (lo > 0 && qAbs(sub[lo - 1]) > m_tolerance *
               (qAbs(diag[lo - 1]) + qAbs(diag[lo])))
            --lo;

        if (lo == hi) {
            /* Eigenvalue converged */
            --hi;
            continue;
        }

        implicitQRStep(diag, sub, Q, lo, hi);
        ++totalIter;

        if (totalIter % 100 == 0)
            emit iterationProgress(totalIter, m_maxIterations * n);
    }

    /* Sort eigenvalues ascending */
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(),
              [&](int a, int b) { return diag[a] < diag[b]; });

    QVector<double> eigenvalues(n);
    QVector<QVector<double>> eigenvectors(n);
    for (int i = 0; i < n; ++i) {
        eigenvalues[i] = diag[order[i]];
        eigenvectors[i].resize(n);
        for (int j = 0; j < n; ++j)
            eigenvectors[i][j] = Q[j][order[i]];
    }

    /* Compute residual */
    m_stats.residual = computeResidual(matrix, eigenvalues, eigenvectors);

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.qrIterations = totalIter;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, totalIter);
    return {eigenvalues, eigenvectors};
}

/* ---- Partial solve ---- */

QVector<double> SymmetricEigenSolver::solvePartial(
    const QVector<QVector<double>>& matrix, int k)
{
    auto all = solve(matrix);
    if (k >= all.size()) return all;
    return all.mid(0, k);
}

/* ---- Residual ---- */

double SymmetricEigenSolver::computeResidual(
    const QVector<QVector<double>>& mat,
    const QVector<double>& eigenvalues,
    const QVector<QVector<double>>& eigenvectors)
{
    int n = mat.size();
    if (n == 0) return 0.0;
    double maxRes = 0.0;

    for (int k = 0; k < n; ++k) {
        /* Compute (A*v - lambda*v) */
        double res = 0.0;
        for (int i = 0; i < n; ++i) {
            double Avi = 0.0;
            for (int j = 0; j < n; ++j)
                Avi += mat[i][j] * eigenvectors[k][j];
            double diff = Avi - eigenvalues[k] * eigenvectors[k][i];
            res += diff * diff;
        }
        maxRes = qMax(maxRes, qSqrt(res));
    }
    return maxRes;
}

/* ---- Reset ---- */

void SymmetricEigenSolver::reset() {}

void SymmetricEigenSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
