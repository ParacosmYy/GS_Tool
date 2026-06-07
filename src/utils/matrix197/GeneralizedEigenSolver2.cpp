/**
 * @file GeneralizedEigenSolver2.cpp
 * @brief GeneralizedEigenSolver2 实现
 *
 * 实现广义特征值求解：QZ分解、实Schur形式、alpha/beta特征值对提取。
 */

#include "utils/matrix197/GeneralizedEigenSolver2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GeneralizedEigenSolver2::GeneralizedEigenSolver2(QObject *parent)
    : QObject(parent) {}
GeneralizedEigenSolver2::~GeneralizedEigenSolver2() = default;

/* ---- Configuration ---- */

void GeneralizedEigenSolver2::setMaxIterations(int iter)
{
    m_maxIter = qMax(10, iter);
}

void GeneralizedEigenSolver2::setTolerance(double tol)
{
    m_tolerance = qMax(1e-15, tol);
}

/* ---- Givens rotation ---- */

void GeneralizedEigenSolver2::givensRotation(
    QVector<QVector<double>>& mat, int row1, int row2, int col,
    double& c, double& s)
{
    double a = mat[row1][col];
    double b = mat[row2][col];
    double r = qSqrt(a * a + b * b);
    if (r < 1e-15) { c = 1.0; s = 0.0; return; }
    c = a / r;
    s = b / r;

    int n = mat[0].size();
    for (int j = col; j < n; ++j) {
        double t1 = c * mat[row1][j] + s * mat[row2][j];
        double t2 = -s * mat[row1][j] + c * mat[row2][j];
        mat[row1][j] = t1;
        mat[row2][j] = t2;
    }
}

/* ---- Hessenberg-triangular reduction ---- */

void GeneralizedEigenSolver2::hessenbergTriangular(
    QVector<QVector<double>>& A, QVector<QVector<double>>& B,
    QVector<QVector<double>>& Q, QVector<QVector<double>>& Z)
{
    int n = A.size();

    // Reduce B to upper triangular using Givens rotations
    for (int j = 0; j < n - 1; ++j) {
        for (int i = n - 1; i > j; --i) {
            if (qAbs(B[i][j]) > m_tolerance) {
                double c, s;
                givensRotation(B, i - 1, i, j, c, s);
                // Apply same rotation to A
                int cols = A[0].size();
                for (int k = 0; k < cols; ++k) {
                    double t1 = c * A[i - 1][k] + s * A[i][k];
                    double t2 = -s * A[i - 1][k] + c * A[i][k];
                    A[i - 1][k] = t1;
                    A[i][k] = t2;
                }
                // Update Q
                for (int k = 0; k < n; ++k) {
                    double t1 = c * Q[k][i - 1] + s * Q[k][i];
                    double t2 = -s * Q[k][i - 1] + c * Q[k][i];
                    Q[k][i - 1] = t1;
                    Q[k][i] = t2;
                }
            }
        }
    }

    // Reduce A to upper Hessenberg
    for (int j = 0; j < n - 2; ++j) {
        for (int i = n - 1; i > j + 1; --i) {
            if (qAbs(A[i][j]) > m_tolerance) {
                double a = A[i - 1][j], b = A[i][j];
                double r = qSqrt(a * a + b * b);
                if (r < 1e-15) continue;
                double c = a / r, s = b / r;

                // Apply rotation to A rows
                for (int k = j; k < n; ++k) {
                    double t1 = c * A[i - 1][k] + s * A[i][k];
                    double t2 = -s * A[i - 1][k] + c * A[i][k];
                    A[i - 1][k] = t1;
                    A[i][k] = t2;
                }
                // Apply rotation to B rows
                for (int k = 0; k < n; ++k) {
                    double t1 = c * B[i - 1][k] + s * B[i][k];
                    double t2 = -s * B[i - 1][k] + c * B[i][k];
                    B[i - 1][k] = t1;
                    B[i][k] = t2;
                }
                // Update Q
                for (int k = 0; k < n; ++k) {
                    double t1 = c * Q[k][i - 1] + s * Q[k][i];
                    double t2 = -s * Q[k][i - 1] + c * Q[k][i];
                    Q[k][i - 1] = t1;
                    Q[k][i] = t2;
                }
                // Apply inverse rotation to A, B columns
                for (int k = 0; k < n; ++k) {
                    double t1 = c * A[k][i - 1] + s * A[k][i];
                    double t2 = -s * A[k][i - 1] + c * A[k][i];
                    A[k][i - 1] = t1;
                    A[k][i] = t2;
                }
                for (int k = 0; k < n; ++k) {
                    double t1 = c * B[k][i - 1] + s * B[k][i];
                    double t2 = -s * B[k][i - 1] + c * B[k][i];
                    B[k][i - 1] = t1;
                    B[k][i] = t2;
                }
                // Update Z
                for (int k = 0; k < n; ++k) {
                    double t1 = c * Z[k][i - 1] + s * Z[k][i];
                    double t2 = -s * Z[k][i - 1] + c * Z[k][i];
                    Z[k][i - 1] = t1;
                    Z[k][i] = t2;
                }
            }
        }
    }
}

/* ---- QZ iteration ---- */

void GeneralizedEigenSolver2::qzIteration(
    QVector<QVector<double>>& A, QVector<QVector<double>>& B,
    QVector<QVector<double>>& Q, QVector<QVector<double>>& Z)
{
    int n = A.size();
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        bool converged = true;

        for (int i = 0; i < n - 1; ++i) {
            // Check if subdiagonal element of A is small enough
            if (qAbs(A[i + 1][i]) > m_tolerance *
                (qAbs(A[i][i]) + qAbs(A[i + 1][i + 1]))) {
                converged = false;
            }
        }
        if (converged) break;

        // Implicit shift using Francis double-step
        for (int i = 0; i < n - 1; ++i) {
            double a11 = A[i][i], a12 = A[i][i + 1];
            double a21 = A[i + 1][i], a22 = A[i + 1][i + 1];
            double b11 = B[i][i], b12 = B[i][i + 1];
            double b22 = B[i + 1][i + 1];

            // Shift: use Wilkinson-type shift
            double shift = (qAbs(b22) > 1e-15) ? a22 / b22 : a22;

            // Compute shift residual
            double x = a11 - shift * b11;
            double y = a21;

            // Givens rotation to zero y
            double r = qSqrt(x * x + y * y);
            if (r < 1e-15) continue;
            double c = x / r, s = y / r;

            // Apply from left to A and B
            for (int j = 0; j < n; ++j) {
                double t1 = c * A[i][j] + s * A[i + 1][j];
                double t2 = -s * A[i][j] + c * A[i + 1][j];
                A[i][j] = t1;
                A[i + 1][j] = t2;

                t1 = c * B[i][j] + s * B[i + 1][j];
                t2 = -s * B[i][j] + c * B[i + 1][j];
                B[i][j] = t1;
                B[i + 1][j] = t2;
            }
            // Update Q
            for (int k = 0; k < n; ++k) {
                double t1 = c * Q[k][i] + s * Q[k][i + 1];
                double t2 = -s * Q[k][i] + c * Q[k][i + 1];
                Q[k][i] = t1;
                Q[k][i + 1] = t2;
            }

            // Apply from right to restore upper triangular B
            for (int j = 0; j < n; ++j) {
                double t1 = c * B[j][i] + s * B[j][i + 1];
                double t2 = -s * B[j][i] + c * B[j][i + 1];
                B[j][i] = t1;
                B[j][i + 1] = t2;

                t1 = c * A[j][i] + s * A[j][i + 1];
                t2 = -s * A[j][i] + c * A[j][i + 1];
                A[j][i] = t1;
                A[j][i + 1] = t2;
            }
            // Update Z
            for (int k = 0; k < n; ++k) {
                double t1 = c * Z[k][i] + s * Z[k][i + 1];
                double t2 = -s * Z[k][i] + c * Z[k][i + 1];
                Z[k][i] = t1;
                Z[k][i + 1] = t2;
            }
        }

        // Deflate converged eigenvalues
        for (int i = 0; i < n - 1; ++i) {
            if (qAbs(A[i + 1][i]) < m_tolerance *
                (qAbs(A[i][i]) + qAbs(A[i + 1][i + 1]) + 1e-30))
                A[i + 1][i] = 0.0;
        }
    }
    m_stats.iterations = iter;
}

/* ---- Extract alpha/beta pairs ---- */

QVector<GeneralizedEigenSolver2::EigenPair>
GeneralizedEigenSolver2::extractEigenvalues(
    const QVector<QVector<double>>& S,
    const QVector<QVector<double>>& T) const
{
    int n = S.size();
    QVector<EigenPair> pairs;
    int i = 0;

    while (i < n) {
        EigenPair p;
        if (i + 1 < n && qAbs(S[i + 1][i]) > m_tolerance) {
            // 2x2 block: complex conjugate pair
            double a = S[i][i], b = S[i][i + 1];
            double c = S[i + 1][i], d = S[i + 1][i + 1];
            double e = T[i][i], f = T[i][i + 1];
            double g = T[i + 1][i + 1];

            double tr = a + d;
            double det = a * d - b * c;

            // alpha = tr ± sqrt(tr^2 - 4*det) / 2
            double disc = tr * tr - 4.0 * det;
            double beta = e * g;
            if (qAbs(beta) < 1e-15) beta = 1e-15;

            p.alpha_r = tr / 2.0;
            p.alpha_i = qSqrt(qAbs(disc)) / 2.0;
            p.beta = beta;
            pairs.append(p);

            p.alpha_i = -p.alpha_i;
            pairs.append(p);
            i += 2;
        } else {
            // 1x1 block: real eigenvalue
            p.alpha_r = S[i][i];
            p.alpha_i = 0.0;
            p.beta = (qAbs(T[i][i]) < 1e-15) ? 1e-15 : T[i][i];
            pairs.append(p);
            i += 1;
        }
    }
    return pairs;
}

/* ---- Solve ---- */

QVector<GeneralizedEigenSolver2::EigenPair>
GeneralizedEigenSolver2::solve(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0 || B.size() != n) return {};

    // Working copies
    auto S = A;
    auto T = B;

    // Initialize Q, Z as identity
    m_Q = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    m_Z = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        m_Q[i][i] = 1.0;
        m_Z[i][i] = 1.0;
    }

    // Step 1: Hessenberg-triangular reduction
    hessenbergTriangular(S, T, m_Q, m_Z);

    // Step 2: QZ iteration
    qzIteration(S, T, m_Q, m_Z);

    // Step 3: Extract eigenvalues
    m_eigenvalues = extractEigenvalues(S, T);

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, m_stats.iterations, timer.elapsed());
    return m_eigenvalues;
}

/* ---- Accessors ---- */

QVector<QVector<double>> GeneralizedEigenSolver2::schurQ() const { return m_Q; }
QVector<QVector<double>> GeneralizedEigenSolver2::schurZ() const { return m_Z; }

/* ---- Reset ---- */

void GeneralizedEigenSolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_eigenvalues.clear();
    m_Q.clear();
    m_Z.clear();
}
