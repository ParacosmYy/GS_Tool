/**
 * @file EigenValueSolver.cpp
 * @brief EigenValueSolver 实现
 *
 * 实现QR算法特征值求解：Householder约化上Hessenberg、
 * Francis隐式双移QR步、Wilkinson位移和2x2块特征值检测。
 */

#include "utils/matrix168/EigenValueSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

EigenValueSolver::EigenValueSolver(QObject* parent)
    : QObject(parent)
{
}

EigenValueSolver::~EigenValueSolver() = default;

void EigenValueSolver::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(10, maxIter);
}

void EigenValueSolver::setConvergenceThreshold(double threshold)
{
    m_threshold = qMax(1e-16, threshold);
}

QVector<double> EigenValueSolver::householderVector(const QVector<double>& x,
                                                     double& beta)
{
    int n = x.size();
    QVector<double> v = x;
    double sigma = 0.0;
    for (int i = 1; i < n; ++i) sigma += x[i] * x[i];

    if (sigma < 1e-30) {
        beta = 0.0;
        v[0] = 1.0;
        return v;
    }

    double mu = qSqrt(x[0] * x[0] + sigma);
    if (x[0] <= 0) {
        v[0] = x[0] - mu;
    } else {
        v[0] = -sigma / (x[0] + mu);
    }

    beta = 2.0 * v[0] * v[0] / (sigma + v[0] * v[0]);
    double scale = v[0];
    for (auto& val : v) val /= scale;
    return v;
}

void EigenValueSolver::reduceToHessenberg(QVector<QVector<double>>& H,
                                          QVector<QVector<double>>& Q) const
{
    int n = H.size();
    Q.resize(n);
    for (int i = 0; i < n; ++i) {
        Q[i].resize(n, 0.0);
        Q[i][i] = 1.0;
    }

    for (int k = 0; k < n - 2; ++k) {
        /* Extract column below diagonal */
        QVector<double> x(n - k - 1);
        for (int i = 0; i < n - k - 1; ++i) x[i] = H[k + 1 + i][k];

        double beta;
        QVector<double> v = householderVector(x, beta);

        /* Apply H <- (I - beta*v*v^T) * H * (I - beta*v*v^T) */
        /* Left: H[k+1:n, k:n] -= beta * v * (v^T * H[k+1:n, k:n]) */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < v.size(); ++i) dot += v[i] * H[k + 1 + i][j];
            for (int i = 0; i < v.size(); ++i) H[k + 1 + i][j] -= beta * v[i] * dot;
        }

        /* Right: H[0:n, k+1:n] -= beta * (H * v) * v^T */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < v.size(); ++j) dot += H[i][k + 1 + j] * v[j];
            for (int j = 0; j < v.size(); ++j) H[i][k + 1 + j] -= beta * dot * v[j];
        }

        /* Accumulate Q */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < v.size(); ++j) dot += Q[i][k + 1 + j] * v[j];
            for (int j = 0; j < v.size(); ++j) Q[i][k + 1 + j] -= beta * dot * v[j];
        }
    }
}

void EigenValueSolver::eigenvalues2x2(double a, double b, double c, double d,
                                      EigenValue& e1, EigenValue& e2) const
{
    double trace = a + d;
    double det = a * d - b * c;
    double disc = trace * trace - 4.0 * det;

    if (disc >= 0) {
        double sq = qSqrt(disc);
        e1 = {0.5 * (trace + sq), 0.0};
        e2 = {0.5 * (trace - sq), 0.0};
    } else {
        double sq = qSqrt(-disc);
        e1 = {0.5 * trace, 0.5 * sq};
        e2 = {0.5 * trace, -0.5 * sq};
    }
}

void EigenValueSolver::francisQRStep(QVector<QVector<double>>& H,
                                     int lo, int hi,
                                     QVector<QVector<double>>& Q) const
{
    int n = H.size();
    /* Wilkinson shift: eigenvalues of bottom-right 2x2 block */
    double a = H[hi - 1][hi - 1], b = H[hi - 1][hi];
    double c = H[hi][hi - 1], d = H[hi][hi];

    EigenValue e1, e2;
    eigenvalues2x2(a, b, c, d, e1, e2);

    /* Choose shift closer to d */
    double s, t;
    if (qAbs(e1.real - d) < qAbs(e2.real - d)) {
        s = e1.real * 2.0;
        t = e1.real * e1.real + e1.imag * e1.imag;
    } else {
        s = e2.real * 2.0;
        t = e2.real * e2.real + e2.imag * e2.imag;
    }

    /* Implicit double shift using first column of (H^2 - s*H + t*I) */
    double x = H[lo][lo] * H[lo][lo] + H[lo][lo + 1] * H[lo + 1][lo] - s * H[lo][lo] + t;
    double y = H[lo + 1][lo] * (H[lo][lo] + H[lo + 1][lo + 1] - s);
    double z = H[lo + 1][lo] * H[lo + 2][lo + 1];

    for (int k = lo; k < hi - 1; ++k) {
        QVector<double> v = {x, y, z};
        double norm = qSqrt(x * x + y * y + z * z);
        if (norm < 1e-30) break;
        for (auto& val : v) val /= norm;

        int r = qMax(lo, k - 1);
        /* Left multiply */
        for (int j = r; j < n; ++j) {
            double dot = v[0] * H[k][j] + v[1] * H[k + 1][j] + v[2] * H[k + 2][j];
            H[k][j] -= 2 * v[0] * dot;
            H[k + 1][j] -= 2 * v[1] * dot;
            H[k + 2][j] -= 2 * v[2] * dot;
        }

        r = qMin(hi, k + 3);
        /* Right multiply */
        for (int i = 0; i <= r; ++i) {
            double dot = v[0] * H[i][k] + v[1] * H[i][k + 1] + v[2] * H[i][k + 2];
            H[i][k] -= 2 * v[0] * dot;
            H[i][k + 1] -= 2 * v[1] * dot;
            H[i][k + 2] -= 2 * v[2] * dot;
        }

        /* Update Q */
        for (int i = 0; i < n; ++i) {
            double dot = v[0] * Q[i][k] + v[1] * Q[i][k + 1] + v[2] * Q[i][k + 2];
            Q[i][k] -= 2 * v[0] * dot;
            Q[i][k + 1] -= 2 * v[1] * dot;
            Q[i][k + 2] -= 2 * v[2] * dot;
        }

        x = H[k + 1][k];
        y = H[k + 2][k];
        if (k < hi - 2) z = H[k + 3][k];
    }

    /* Final 2x2 Givens rotation */
    {
        int k = hi - 1;
        double norm = qSqrt(x * x + y * y);
        if (norm > 1e-30) {
            double c2 = x / norm, s2 = y / norm;
            for (int j = qMax(lo, k - 1); j < n; ++j) {
                double t1 = H[k][j], t2 = H[k + 1][j];
                H[k][j] = c2 * t1 + s2 * t2;
                H[k + 1][j] = -s2 * t1 + c2 * t2;
            }
            for (int i = 0; i <= hi; ++i) {
                double t1 = H[i][k], t2 = H[i][k + 1];
                H[i][k] = c2 * t1 + s2 * t2;
                H[i][k + 1] = -s2 * t1 + c2 * t2;
            }
            for (int i = 0; i < n; ++i) {
                double t1 = Q[i][k], t2 = Q[i][k + 1];
                Q[i][k] = c2 * t1 + s2 * t2;
                Q[i][k + 1] = -s2 * t1 + c2 * t2;
            }
        }
    }
}

QVector<EigenValueSolver::EigenValue> EigenValueSolver::solve(
    const QVector<QVector<double>>& matrix)
{
    QVector<QVector<double>> dummy;
    return solveWithVectors(matrix, dummy);
}

QVector<EigenValueSolver::EigenValue> EigenValueSolver::solveWithVectors(
    const QVector<QVector<double>>& matrix,
    QVector<QVector<double>>& eigenVectors)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return QVector<EigenValue>();

    /* Copy matrix */
    QVector<QVector<double>> H = matrix;
    for (int i = 0; i < n; ++i) {
        H[i].resize(n, 0.0);
    }

    /* Reduce to upper Hessenberg form */
    QVector<QVector<double>> Q;
    reduceToHessenberg(H, Q);

    /* QR iteration with implicit double shifts */
    int hi = n - 1;
    int totalIter = 0;

    while (hi > 0 && totalIter < m_maxIterations) {
        /* Check for deflation: sub-diagonal element near zero */
        int lo = 0;
        for (int i = hi; i > 0; --i) {
            if (qAbs(H[i][i - 1]) <= m_threshold * (qAbs(H[i][i]) + qAbs(H[i - 1][i - 1]))) {
                H[i][i - 1] = 0.0;
                lo = i;
                break;
            }
        }

        if (lo == hi) {
            /* 1x1 block: eigenvalue found */
            hi--;
        } else if (lo == hi - 1) {
            /* 2x2 block: compute eigenvalues directly */
            hi -= 2;
        } else {
            francisQRStep(H, lo, hi, Q);
            totalIter++;
        }
    }

    /* Extract eigenvalues from quasi-triangular form */
    QVector<EigenValue> eigenvalues;
    int i = 0;
    while (i < n) {
        if (i == n - 1 || qAbs(H[i + 1][i]) <= m_threshold *
            (qAbs(H[i][i]) + qAbs(H[i + 1][i + 1]))) {
            /* Real eigenvalue */
            eigenvalues.append({H[i][i], 0.0});
            i++;
        } else {
            /* 2x2 block: complex conjugate pair */
            EigenValue e1, e2;
            eigenvalues2x2(H[i][i], H[i][i + 1], H[i + 1][i], H[i + 1][i + 1], e1, e2);
            eigenvalues.append(e1);
            eigenvalues.append(e2);
            i += 2;
        }
    }

    /* Q now contains eigenvectors as columns */
    eigenVectors = Q;

    m_stats.totalSolves++;
    m_stats.totalQrIterations += totalIter;
    m_stats.lastMatrixSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(n, eigenvalues.size());
    return eigenvalues;
}

void EigenValueSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
