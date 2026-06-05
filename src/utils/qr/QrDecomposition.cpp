/**
 * @file QrDecomposition.cpp
 * @brief QR分解 — Householder反射矩阵分解
 */

#include "QrDecomposition.h"
#include <QElapsedTimer>
#include <cmath>

QrDecomposition::QrDecomposition(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QPair<QVector<QVector<double>>, QVector<QVector<double>>>
QrDecomposition::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size();
    if (m == 0) return {{}, {}};
    int n = A[0].size();

    /* R = A的拷贝, Q = 单位矩阵 */
    QVector<QVector<double>> R = A;
    QVector<QVector<double>> Q(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) Q[i][i] = 1.0;

    int steps = qMin(m - 1, n);
    for (int k = 0; k < steps; ++k) {
        /* 提取列向量x = R[k:m, k] */
        QVector<double> x(m - k);
        for (int i = k; i < m; ++i) x[i - k] = R[i][k];

        /* 计算Householder向量 */
        double normX = 0.0;
        for (double v : x) normX += v * v;
        normX = std::sqrt(normX);

        if (normX < 1e-15) continue;

        double sign = (x[0] >= 0) ? 1.0 : -1.0;
        x[0] += sign * normX;

        double normV = 0.0;
        for (double v : x) normV += v * v;
        if (normV < 1e-15) continue;

        /* H = I - 2*v*v^T / (v^T*v) */
        /* R = H * R */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < x.size(); ++i) dot += x[i] * R[k + i][j];
            for (int i = 0; i < x.size(); ++i) R[k + i][j] -= 2.0 * dot * x[i] / normV;
        }

        /* Q = Q * H */
        for (int j = 0; j < m; ++j) {
            double dot = 0.0;
            for (int i = 0; i < x.size(); ++i) dot += Q[j][k + i] * x[i];
            for (int i = 0; i < x.size(); ++i) Q[j][k + i] -= 2.0 * dot * x[i] / normV;
        }
    }

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(m, n);
    return {Q, R};
}

QVector<double> QrDecomposition::solveLeastSquares(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    auto [Q, R] = decompose(A);
    if (Q.isEmpty()) return {};

    int m = A.size();
    int n = A[0].size();

    /* Q^T * b */
    QVector<double> Qtb(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            Qtb[i] += Q[i][j] * b[j];
        }
    }

    /* 回代 R * x = Qtb */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = Qtb[i];
        for (int j = i + 1; j < n; ++j) {
            x[i] -= R[i][j] * x[j];
        }
        if (std::abs(R[i][i]) > 1e-15) {
            x[i] /= R[i][i];
        }
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves);

    return x;
}

double QrDecomposition::determinant(const QVector<QVector<double>>& A)
{
    auto [Q, R] = decompose(A);
    if (R.isEmpty()) return 0.0;

    int n = R.size();
    double det = 1.0;
    for (int i = 0; i < n; ++i) {
        det *= R[i][i];
    }
    return det;
}

int QrDecomposition::rank(const QVector<QVector<double>>& A, double tolerance) const
{
    if (A.isEmpty()) return 0;
    int n = A[0].size();
    int m = A.size();
    int r = 0;

    QrDecomposition* self = const_cast<QrDecomposition*>(this);
    auto [Q, R] = self->decompose(A);

    for (int i = 0; i < qMin(m, n); ++i) {
        if (std::abs(R[i][i]) > tolerance) r++;
    }
    return r;
}

void QrDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
