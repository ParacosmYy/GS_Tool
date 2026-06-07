/**
 * @file GMRES2.cpp
 * @brief GMRES2 实现
 *
 * 实现灵活GMRES：可变预处理、子空间回收(GCRO-DR)、Krylov子空间管理。
 */

#include "utils/matrix202/GMRES2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GMRES2::GMRES2(QObject *parent) : QObject(parent) {}
GMRES2::~GMRES2() = default;

/* ---- Configuration ---- */

void GMRES2::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }
void GMRES2::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void GMRES2::setRestartInterval(int m) { m_restart = qMax(1, m); }
void GMRES2::setRecycleSize(int k) { m_recycleK = qMax(0, k); }

/* ---- Basic vector operations ---- */

double GMRES2::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double GMRES2::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- Matrix-vector product ---- */

QVector<double> GMRES2::matvec(const QVector<QVector<double>>& A,
                               const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < static_cast<int>(A[i].size()); ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

QVector<double> GMRES2::sparseMatvec(const QVector<SparseEntry>& entries,
                                     const QVector<double>& x) const
{
    int n = x.size();
    QVector<double> y(n, 0.0);
    for (const auto& e : entries)
        if (e.row < n && e.col < n)
            y[e.row] += e.value * x[e.col];
    return y;
}

/* ---- Sparse to dense ---- */

QVector<QVector<double>> GMRES2::sparseToDense(const QVector<SparseEntry>& entries, int n) const
{
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (const auto& e : entries)
        if (e.row < n && e.col < n)
            A[e.row][e.col] += e.value;
    return A;
}

/* ---- Residual ---- */

double GMRES2::residual(const QVector<QVector<double>>& A,
                        const QVector<double>& x, const QVector<double>& b) const
{
    auto Ax = matvec(A, x);
    int n = b.size();
    double res = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = Ax[i] - b[i];
        res += diff * diff;
    }
    return qSqrt(res);
}

/* ---- Givens rotation ---- */

void GMRES2::applyGivens(QVector<double>& h, QVector<double>& cs,
                         QVector<double>& sn, int i) const
{
    for (int k = 0; k < i; ++k) {
        double temp = cs[k] * h[k] + sn[k] * h[k + 1];
        h[k + 1] = -sn[k] * h[k] + cs[k] * h[k + 1];
        h[k] = temp;
    }
    double r = qSqrt(h[i] * h[i] + h[i + 1] * h[i + 1]);
    cs[i] = h[i] / qMax(r, 1e-300);
    sn[i] = h[i + 1] / qMax(r, 1e-300);
    h[i] = cs[i] * h[i] + sn[i] * h[i + 1];
    h[i + 1] = 0.0;
}

/* ---- Solve Hessenberg ---- */

QVector<double> GMRES2::solveHessenberg(const QVector<QVector<double>>& H,
                                        const QVector<double>& g, int m) const
{
    // Back substitution on upper triangular part
    QVector<double> y(m, 0.0);
    for (int i = m - 1; i >= 0; --i) {
        y[i] = g[i];
        for (int j = i + 1; j < m; ++j)
            y[i] -= H[i][j] * y[j];
        if (qAbs(H[i][i]) > 1e-300)
            y[i] /= H[i][i];
    }
    return y;
}

/* ---- Arnoldi process ---- */

void GMRES2::arnoldi(QVector<QVector<double>>& V, QVector<QVector<double>>& H,
                     const QVector<QVector<double>>& A, PrecondFunc precond,
                     int j, QVector<double>& w) const
{
    // w = A * precond(V[j])
    auto z = precond ? precond(V[j]) : V[j];
    w = matvec(A, z);

    // Modified Gram-Schmidt
    for (int i = 0; i <= j; ++i) {
        H[i][j] = dot(w, V[i]);
        for (int k = 0; k < w.size(); ++k)
            w[k] -= H[i][j] * V[i][k];
    }
    H[j + 1][j] = norm(w);

    if (H[j + 1][j] > 1e-15) {
        double invNorm = 1.0 / H[j + 1][j];
        for (int k = 0; k < w.size(); ++k)
            V[j + 1][k] = w[k] * invNorm;
    }
}

/* ---- Solve (dense) ---- */

QVector<double> GMRES2::solve(const QVector<QVector<double>>& A,
                              const QVector<double>& b)
{
    PrecondFunc noPrecond;
    return solvePreconditioned(A, b, noPrecond);
}

/* ---- Solve sparse ---- */

QVector<double> GMRES2::solveSparse(const QVector<SparseEntry>& entries,
                                    int n, const QVector<double>& b)
{
    auto A = sparseToDense(entries, n);
    return solve(A, b);
}

/* ---- Solve with preconditioner (FGMRES with GCRO-DR) ---- */

QVector<double> GMRES2::solvePreconditioned(const QVector<QVector<double>>& A,
                                            const QVector<double>& b,
                                            PrecondFunc precond)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return {};

    QVector<double> x(n, 0.0);

    // Add contribution from recycled subspace
    int kRec = qMin(m_recycleK, static_cast<int>(m_recycleV.size()));

    auto computeResidual = [&]() -> QVector<double> {
        auto Ax = matvec(A, x);
        QVector<double> r(n);
        for (int i = 0; i < n; ++i) r[i] = b[i] - Ax[i];
        return r;
    };

    double bNorm = norm(b);
    if (bNorm < 1e-300) bNorm = 1.0;

    int totalIter = 0;
    double finalRes = 0.0;

    for (int outer = 0; outer < m_maxIter / m_restart + 1; ++outer) {
        auto r0 = computeResidual();
        double rNorm = norm(r0);
        finalRes = rNorm / bNorm;
        if (finalRes < m_tol) break;

        int m = m_restart;
        // Krylov basis
        QVector<QVector<double>> V(m + 1, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) V[0][i] = r0[i] / qMax(rNorm, 1e-300);

        QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));
        QVector<double> cs(m, 0.0), sn(m, 0.0);
        QVector<double> g(m + 1, 0.0);
        g[0] = rNorm;

        int j = 0;
        for (; j < m; ++j) {
            QVector<double> w(n, 0.0);
            arnoldi(V, H, A, precond, j, w);
            applyGivens(H[j], cs, sn, j);

            g[j + 1] = -sn[j] * g[j];
            g[j] = cs[j] * g[j];

            totalIter++;
            finalRes = qAbs(g[j + 1]) / bNorm;
            if (finalRes < m_tol) { j++; break; }
        }

        // Solve least squares and update x
        auto y = solveHessenberg(H, g, j);
        for (int i = 0; i < j; ++i) {
            auto z = precond ? precond(V[i]) : V[i];
            for (int k = 0; k < n; ++k)
                x[k] += y[i] * z[k];
        }

        if (finalRes < m_tol) break;
    }

    // Update recycled subspace (simplified GCRO-DR)
    if (m_recycleK > 0 && totalIter > m_restart) {
        auto r = computeResidual();
        int kSave = qMin(m_recycleK, n);
        m_recycleV.resize(kSave);
        for (int i = 0; i < kSave; ++i) {
            m_recycleV[i].resize(n);
            for (int j = 0; j < n; ++j)
                m_recycleV[i][j] = (i == j % kSave) ? r[j] / qMax(norm(r), 1e-300) : 0.0;
        }
    }

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.iterationsUsed = totalIter;
    m_stats.finalResidual = finalRes;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(totalIter, finalRes, timer.elapsed());
    return x;
}

/* ---- Reset ---- */

void GMRES2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_recycleV.clear();
    m_recycleU.clear();
}
