/**
 * @file GeneralizedEigenSolver.cpp
 * @brief GeneralizedEigenSolver 实现
 *
 * 实现广义特征值QZ分解：Hessenberg-三角归约、QZ步进、降阶、Schur分解。
 */

#include "utils/matrix181/GeneralizedEigenSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GeneralizedEigenSolver::GeneralizedEigenSolver(QObject *parent)
    : QObject(parent)
{
}

GeneralizedEigenSolver::~GeneralizedEigenSolver() = default;

/* ---- Givens rotation ---- */

void GeneralizedEigenSolver::applyGivens(QVector<QVector<double>>& M,
                                           int i, int j,
                                           double c, double s, bool fromLeft)
{
    int n = M.size();
    if (fromLeft) {
        for (int k = 0; k < n; ++k) {
            double mi = M[i][k], mj = M[j][k];
            M[i][k] = c * mi + s * mj;
            M[j][k] = -s * mi + c * mj;
        }
    } else {
        for (int k = 0; k < n; ++k) {
            double mi = M[k][i], mj = M[k][j];
            M[k][i] = c * mi + s * mj;
            M[k][j] = -s * mi + c * mj;
        }
    }
}

/* ---- Hessenberg-triangular reduction ---- */

void GeneralizedEigenSolver::hessenbergTriangular(
    QVector<QVector<double>>& A, QVector<QVector<double>>& B)
{
    int n = A.size();

    /* Reduce B to upper triangular using Givens rotations */
    for (int j = 0; j < n; ++j) {
        for (int i = n - 1; i > j; --i) {
            if (qAbs(B[i][j]) > 1e-15) {
                double r = qSqrt(B[i][j] * B[i][j] + B[i - 1][j] * B[i - 1][j]);
                double c = B[i - 1][j] / r;
                double s = B[i][j] / r;

                /* Apply to B from left */
                applyGivens(B, i - 1, i, c, s, true);
                /* Apply to A from left */
                applyGivens(A, i - 1, i, c, s, true);
                /* Apply to Q from right */
                applyGivens(m_Q, i - 1, i, c, s, false);
            }
        }
    }

    /* Reduce A to upper Hessenberg */
    for (int j = 0; j < n - 2; ++j) {
        for (int i = n - 1; i > j + 1; --i) {
            if (qAbs(A[i][j]) > 1e-15) {
                double r = qSqrt(A[i][j] * A[i][j] + A[i - 1][j] * A[i - 1][j]);
                double c = A[i - 1][j] / r;
                double s = A[i][j] / r;

                applyGivens(A, i - 1, i, c, s, true);
                applyGivens(B, i - 1, i, c, s, true);
                applyGivens(m_Z, i - 1, i, c, s, false);
            }
        }
    }
}

/* ---- QZ step ---- */

void GeneralizedEigenSolver::qzStep(QVector<QVector<double>>& A,
                                      QVector<QVector<double>>& B,
                                      int lo, int hi)
{
    int n = A.size();
    if (hi <= lo) return;

    /* Francis double-shift QZ step */
    /* Compute shift from trailing 2x2 of A and B */
    double a11 = A[hi - 1][hi - 1], a12 = A[hi - 1][hi];
    double a21 = A[hi][hi - 1], a22 = A[hi][hi];
    double b11 = B[hi - 1][hi - 1], b12 = B[hi - 1][hi];
    double b22 = B[hi][hi];

    /* Simplified: use Wilkinson-type shift */
    double sVal = qAbs(a22 * b11 - a11 * b22) + qAbs(a21 * b12);
    double x = A[lo + 1][lo] * (B[lo][lo] / qMax(1e-15, qAbs(B[lo + 1][lo + 1])));
    double z = A[lo + 1][lo] * (B[lo + 1][lo] / qMax(1e-15, qAbs(B[lo + 1][lo + 1])));

    /* Chase the bulge */
    for (int k = lo; k < hi - 1; ++k) {
        /* Determine Givens rotation to zero out bottom element */
        double r = qSqrt(x * x + z * z);
        double c = (r > 1e-15) ? x / r : 1.0;
        double s = (r > 1e-15) ? z / r : 0.0;

        /* Apply from left and right */
        int row1 = k, row2 = k + 1;
        applyGivens(A, row1, row2, c, s, true);
        applyGivens(B, row1, row2, c, s, true);
        applyGivens(m_Q, row1, row2, c, s, false);

        applyGivens(A, row1, row2, c, -s, false);
        applyGivens(B, row1, row2, c, -s, false);
        applyGivens(m_Z, row1, row2, c, -s, false);

        /* Update x, z for next iteration */
        if (k + 2 <= hi) {
            x = A[k + 1][k];
            z = A[k + 2][k];
        }
    }

    /* Deflation check */
    for (int i = lo; i < hi; ++i) {
        if (qAbs(A[i + 1][i]) < 1e-10 * (qAbs(A[i][i]) + qAbs(A[i + 1][i + 1]))) {
            A[i + 1][i] = 0.0;
        }
        if (qAbs(B[i + 1][i + 1]) < 1e-15 && qAbs(B[i][i]) < 1e-15) {
            B[i + 1][i + 1] = 1e-15;
        }
    }
}

/* ---- Main solver ---- */

QVector<GeneralizedEigenSolver::EigenValue> GeneralizedEigenSolver::solve(
    const QVector<QVector<double>>& A, const QVector<QVector<double>>& B)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0) return {};

    /* Initialize working copies */
    m_S = A;
    m_T = B;

    /* Initialize orthogonal matrices to identity */
    m_Q = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    m_Z = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) { m_Q[i][i] = 1.0; m_Z[i][i] = 1.0; }

    /* Step 1: Hessenberg-triangular reduction */
    hessenbergTriangular(m_S, m_T);

    /* Step 2: QZ iteration */
    int maxIter = 30 * n;
    int iter = 0;
    int hi = n - 1;

    while (hi > 0 && iter < maxIter) {
        /* Deflation check: find largest unreduced block */
        while (hi > 0 && qAbs(m_S[hi][hi - 1]) <
               1e-10 * (qAbs(m_S[hi - 1][hi - 1]) + qAbs(m_S[hi][hi]))) {
            m_S[hi][hi - 1] = 0.0;
            hi--;
        }
        if (hi <= 0) break;

        /* Find lo */
        int lo = hi - 1;
        while (lo > 0 && qAbs(m_S[lo][lo - 1]) >=
               1e-10 * (qAbs(m_S[lo - 1][lo - 1]) + qAbs(m_S[lo][lo]))) {
            lo--;
        }

        qzStep(m_S, m_T, lo, hi);
        iter++;
    }

    /* Step 3: Extract eigenvalues from quasi-triangular S and upper-tri T */
    QVector<EigenValue> eigenvalues;
    int i = 0;
    while (i < n) {
        EigenValue ev;
        if (i + 1 < n && qAbs(m_S[i + 1][i]) > 1e-10) {
            /* 2x2 block: complex conjugate pair */
            double a = m_S[i][i], b = m_S[i][i + 1];
            double c = m_S[i + 1][i], d = m_S[i + 1][i + 1];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            if (disc < 0) {
                ev.alphaRe = tr / 2.0;
                ev.alphaIm = qSqrt(-disc) / 2.0;
                ev.beta = qMax(qAbs(m_T[i][i]), qAbs(m_T[i + 1][i + 1]));
                eigenvalues.append(ev);
                ev.alphaIm = -ev.alphaIm;
                eigenvalues.append(ev);
            } else {
                ev.alphaRe = (tr + qSqrt(disc)) / 2.0;
                ev.beta = qMax(1e-15, qAbs(m_T[i][i]));
                eigenvalues.append(ev);
                ev.alphaRe = (tr - qSqrt(disc)) / 2.0;
                eigenvalues.append(ev);
            }
            i += 2;
        } else {
            /* 1x1 block */
            ev.alphaRe = m_S[i][i];
            ev.beta = qMax(1e-15, qAbs(m_T[i][i]));
            eigenvalues.append(ev);
            i++;
        }
    }

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.qzIterations = iter;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, iter);
    return eigenvalues;
}

/* ---- Accessors ---- */

QPair<QVector<QVector<double>>, QVector<QVector<double>>>
GeneralizedEigenSolver::schurForm() const { return {m_S, m_T}; }

QPair<QVector<QVector<double>>, QVector<QVector<double>>>
GeneralizedEigenSolver::qzMatrices() const { return {m_Q, m_Z}; }

void GeneralizedEigenSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
