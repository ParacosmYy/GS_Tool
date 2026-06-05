/**
 * @file SymmetricEigen9.cpp
 * @brief 对称矩阵特征值分解实现
 *
 * 实现对称矩阵的三对角化和QR迭代求解特征值/特征向量，
 * 适用于中小规模稠密对称矩阵。
 */

#include "utils/matrix76/SymmetricEigen9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

SymmetricEigen9::SymmetricEigen9(QObject* parent) : QObject(parent) {}

void SymmetricEigen9::setMatrix(const QVector<QVector<double>>& A) {
    m_n = A.size();
    m_A = A;
    m_eigenvalues.clear();
    m_eigvecs.clear();
}

/**
 * @brief 求解特征值和特征向量
 * @return 是否成功
 */
bool SymmetricEigen9::solve() {
    QElapsedTimer timer; timer.start();
    if (m_n == 0 || m_A.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalDecompositions++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;
        return false;
    }
    tridiagonalize();
    qrIteration();

    qint64 elapsed = timer.elapsed();
    m_stats.totalDecompositions++;
    m_stats.totalDimensions += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    int posCount = 0;
    for (double v : m_eigenvalues) { if (v > 0) posCount++; }
    emit decompositionCompleted(m_n, posCount);
    return true;
}

int SymmetricEigen9::rank(double tol) const {
    int r = 0;
    for (double v : m_eigenvalues) { if (qAbs(v) > tol) r++; }
    return r;
}

void SymmetricEigen9::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief Householder三对角化
 */
void SymmetricEigen9::tridiagonalize() {
    int n = m_n;
    m_eigvecs.resize(n);
    for (int i = 0; i < n; ++i) {
        m_eigvecs[i].resize(n, 0.0);
        m_eigvecs[i][i] = 1.0;
    }

    QVector<QVector<double>> T = m_A;

    for (int k = 0; k < n - 2; ++k) {
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i) norm += T[i][k] * T[i][k];
        norm = qSqrt(norm);
        if (norm < 1e-15) continue;

        double alpha = (T[k + 1][k] >= 0) ? -norm : norm;
        double beta = norm * (norm + qAbs(T[k + 1][k]));
        T[k + 1][k] -= alpha;
        if (qAbs(beta) < 1e-300) continue;

        QVector<double> p(n, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int j = k + 1; j < n; ++j) p[i] += T[i][j] * T[j][k];
            p[i] /= beta;
        }

        double K = 0.0;
        for (int i = k + 1; i < n; ++i) K += T[i][k] * p[i];
        K /= (2.0 * beta);

        QVector<double> q = p;
        for (int i = k + 1; i < n; ++i) q[i] -= K * T[i][k];

        for (int i = k + 1; i < n; ++i)
            for (int j = k + 1; j < n; ++j)
                T[i][j] -= T[i][k] * q[j] + q[i] * T[j][k];

        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += m_eigvecs[i][j] * T[j][k];
            for (int j = k + 1; j < n; ++j) m_eigvecs[i][j] -= dot * T[j][k] / beta;
        }

        T[k + 1][k] = alpha; T[k][k + 1] = alpha;
        for (int i = k + 2; i < n; ++i) { T[i][k] = 0.0; T[k][i] = 0.0; }
    }
    m_A = T;
}

/**
 * @brief 带Wilkinson位移的QR迭代
 */
void SymmetricEigen9::qrIteration() {
    int n = m_n;
    if (n == 0) return;

    QVector<double> diag(n), subdiag(n - 1);
    for (int i = 0; i < n; ++i) diag[i] = m_A[i][i];
    for (int i = 0; i < n - 1; ++i) subdiag[i] = m_A[i][i + 1];

    for (int iter = 0; iter < 100 * n; ++iter) {
        bool converged = true;
        for (int i = 0; i < n - 1; ++i)
            if (qAbs(subdiag[i]) > 1e-12 * (qAbs(diag[i]) + qAbs(diag[i + 1]))) { converged = false; break; }
        if (converged) break;

        double d = (diag[n - 2] - diag[n - 1]) * 0.5;
        double sign = (d >= 0) ? 1.0 : -1.0;
        double mu = diag[n - 1] - subdiag[n - 2] * subdiag[n - 2] / (d + sign * qSqrt(d * d + subdiag[n - 2] * subdiag[n - 2]));

        double x = diag[0] - mu, z = subdiag[0];
        for (int i = 0; i < n - 1; ++i) {
            double r = qSqrt(x * x + z * z);
            if (r < 1e-30) break;
            double c = x / r, s = z / r;
            double w = c * subdiag[i] + s * diag[i + 1];
            diag[i + 1] = -s * subdiag[i] + c * diag[i + 1];
            subdiag[i] = w;

            for (int j = 0; j < n; ++j) {
                double v1 = m_eigvecs[j][i], v2 = m_eigvecs[j][i + 1];
                m_eigvecs[j][i] = c * v1 + s * v2;
                m_eigvecs[j][i + 1] = -s * v1 + c * v2;
            }
            if (i < n - 2) { x = subdiag[i]; z = s * subdiag[i + 1]; subdiag[i + 1] = c * subdiag[i + 1]; }
        }
    }

    m_eigenvalues = diag;
    QVector<int> idx(n);
    for (int i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return diag[a] > diag[b]; });

    QVector<double> sv(n);
    QVector<QVector<double>> se(n, QVector<double>(n));
    for (int i = 0; i < n; ++i) {
        sv[i] = m_eigenvalues[idx[i]];
        for (int j = 0; j < n; ++j) se[j][i] = m_eigvecs[j][idx[i]];
    }
    m_eigenvalues = sv;
    m_eigvecs = se;
}
