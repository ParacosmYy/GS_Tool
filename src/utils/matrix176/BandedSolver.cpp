/**
 * @file BandedSolver.cpp
 * @brief BandedSolver 实现
 *
 * 实现带状矩阵求解：紧凑存储LU分解、Thomas三对角算法、前代/回代。
 */

#include "utils/matrix176/BandedSolver.h"

#include <QElapsedTimer>

/* ---- Construction / Destruction ---- */

BandedSolver::BandedSolver(QObject *parent)
    : QObject(parent)
{
}

BandedSolver::~BandedSolver() = default;

/* ---- Pack full matrix to banded storage ---- */

QVector<QVector<double>> BandedSolver::packBanded(
    const QVector<QVector<double>>& full, int kl, int ku)
{
    int n = full.size();
    int bw = kl + ku + 1;
    QVector<QVector<double>> packed(n, QVector<double>(bw, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = qMax(0, i - kl); j <= qMin(n - 1, i + ku); ++j) {
            packed[i][j - i + ku] = full[i][j];
        }
    }
    return packed;
}

/* ---- Thomas algorithm for tridiagonal systems ---- */

QVector<double> BandedSolver::thomas(const QVector<double>& lower,
                                      const QVector<double>& main,
                                      const QVector<double>& upper,
                                      const QVector<double>& rhs)
{
    int n = main.size();
    if (n == 0) return {};

    QVector<double> c(upper);
    QVector<double> d(rhs);
    QVector<double> x(n);

    /* Forward elimination */
    for (int i = 1; i < n; ++i) {
        double m = lower[i - 1] / d[i - 1];
        d[i] -= m * c[i - 1];
        c[i - 1] = 0.0; /* consumed */

        /* rhs elimination stored in d */
        double newDi = main[i] - m * c[i - 1];
        /* Actually, we modify d directly */
        d[i] = rhs[i] - m * d[i - 1];
    }

    /* Rebuild properly */
    QVector<double> cp(n), dp(n);
    cp[0] = upper.isEmpty() ? 0.0 : upper[0] / main[0];
    dp[0] = rhs[0] / main[0];

    for (int i = 1; i < n; ++i) {
        double denom = main[i] - (i - 1 < lower.size() ? lower[i - 1] : 0.0) * cp[i - 1];
        if (qFabs(denom) < 1e-15) return {}; /* Singular */
        cp[i] = (i < upper.size() ? upper[i] : 0.0) / denom;
        dp[i] = (rhs[i] - (i - 1 < lower.size() ? lower[i - 1] : 0.0) * dp[i - 1]) / denom;
    }

    /* Back substitution */
    x[n - 1] = dp[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = dp[i] - cp[i] * x[i + 1];

    return x;
}

/* ---- Banded LU decomposition (no pivoting) ---- */

bool BandedSolver::luDecompose(QVector<QVector<double>>& A, int kl, int ku) const
{
    int n = A.size();
    int bw = kl + ku + 1;

    for (int k = 0; k < n; ++k) {
        /* Diagonal element position in packed row */
        int diagIdx = ku; /* column ku in packed row is the diagonal */

        if (qFabs(A[k][diagIdx]) < 1e-15) return false;

        /* Eliminate entries below diagonal */
        for (int i = k + 1; i <= qMin(n - 1, k + kl); ++i) {
            int rowOffset = i - k;
            int srcCol = diagIdx - rowOffset;
            if (srcCol < 0 || srcCol >= bw) continue;

            double factor = A[i][srcCol + (k - i + ku)] / A[k][diagIdx];
            /* Actually: A[i][k-i+ku] / A[k][ku] */
            int colForK = ku; /* k in row k is at index ku */
            int colForI_k = k - i + ku; /* k in row i */

            if (colForI_k < 0 || colForI_k >= bw) continue;

            factor = A[i][colForI_k] / A[k][colForK];

            /* Update row i */
            for (int j = qMax(k, i - ku); j <= qMin(n - 1, i + ku); ++j) {
                int colInK = j - k + ku;
                int colInI = j - i + ku;
                if (colInK >= 0 && colInK < bw && colInI >= 0 && colInI < bw)
                    A[i][colInI] -= factor * A[k][colInK];
            }

            /* Store multiplier */
            A[i][colForI_k] = factor;
        }
    }
    return true;
}

/* ---- Forward substitution (Ly = b) ---- */

QVector<double> BandedSolver::forwardSub(const QVector<QVector<double>>& A,
                                          const QVector<double>& b, int kl) const
{
    int n = A.size();
    QVector<double> y = b;

    for (int i = 0; i < n; ++i) {
        for (int k = qMax(0, i - kl); k < i; ++k) {
            int col = k - i + /* ku */ 0; /* simplified */
            /* L factor stored below diagonal */
            int colIdx = k - i + (A[i].size() - 1) / 2;
            if (colIdx >= 0 && colIdx < A[i].size())
                y[i] -= A[i][colIdx] * y[k];
        }
    }
    return y;
}

/* ---- Backward substitution (Ux = y) ---- */

QVector<double> BandedSolver::backwardSub(const QVector<QVector<double>>& A,
                                           const QVector<double>& y, int kl, int ku) const
{
    int n = A.size();
    int bw = kl + ku + 1;
    QVector<double> x = y;

    for (int i = n - 1; i >= 0; --i) {
        for (int j = i + 1; j <= qMin(n - 1, i + ku); ++j) {
            int colIdx = j - i + ku;
            if (colIdx < bw)
                x[i] -= A[i][colIdx] * x[j];
        }
        int diagIdx = ku;
        if (qFabs(A[i][diagIdx]) < 1e-15) return {};
        x[i] /= A[i][diagIdx];
    }
    return x;
}

/* ---- Main solver ---- */

QVector<double> BandedSolver::solve(const QVector<QVector<double>>& A,
                                     const QVector<double>& b,
                                     int kl, int ku)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0) return {};

    /* Special case: tridiagonal */
    if (kl == 1 && ku == 1 && n > 1) {
        QVector<double> lower(n - 1), main(n), upper(n - 1);
        for (int i = 0; i < n; ++i) {
            main[i] = A[i].size() > 1 ? A[i][1] : A[i][0];
            if (i < n - 1) {
                upper[i] = A[i].size() > 2 ? A[i][2] : 0.0;
                lower[i] = A[i + 1].size() > 0 ? A[i + 1][0] : 0.0;
            }
        }
        QVector<double> result = thomas(lower, main, upper, b);

        m_stats.totalSolves++;
        m_stats.lastSize = n;
        m_stats.lastBandwidth = kl + ku + 1;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
            ? m_timeSum / m_stats.totalSolves : 0.0;
        emit solveCompleted(n, kl + ku + 1);
        return result;
    }

    /* General banded: LU decomposition */
    QVector<QVector<double>> lu = A;
    if (!luDecompose(lu, kl, ku)) return {};

    QVector<double> y = forwardSub(lu, b, kl);
    QVector<double> x = backwardSub(lu, y, kl, ku);

    m_stats.totalSolves++;
    m_stats.lastSize = n;
    m_stats.lastBandwidth = kl + ku + 1;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(n, kl + ku + 1);
    return x;
}

/* ---- Statistics ---- */

void BandedSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
