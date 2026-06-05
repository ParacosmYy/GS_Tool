/**
 * @file SingularValueDecomposition.cpp
 * @brief SVD奇异值分解 — 双边Jacobi旋转法
 */

#include "SingularValueDecomposition.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

SingularValueDecomposition::SingularValueDecomposition(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

SingularValueDecomposition::SVDResult
SingularValueDecomposition::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size();
    int n = A.isEmpty() ? 0 : A[0].size();

    /* B = A^T * A 用于计算V */
    QVector<QVector<double>> B(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < m; ++k)
                B[i][j] += A[k][i] * A[k][j];

    /* V初始化为单位矩阵 */
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    /* 双边Jacobi旋转 */
    const int maxIter = 100;
    for (int iter = 0; iter < maxIter; ++iter) {
        double offDiag = 0.0;
        for (int p = 0; p < n; ++p)
            for (int q = p + 1; q < n; ++q)
                offDiag += B[p][q] * B[p][q];

        if (std::sqrt(offDiag) < 1e-10) break;

        for (int p = 0; p < n - 1; ++p) {
            for (int q = p + 1; q < n; ++q) {
                if (std::abs(B[p][q]) < 1e-15) continue;

                double app = B[p][p], aqq = B[q][q], apq = B[p][q];
                double tau = (aqq - app) / (2.0 * apq);
                double t = (tau >= 0) ? 1.0 / (tau + std::sqrt(1.0 + tau * tau))
                                      : -1.0 / (-tau + std::sqrt(1.0 + tau * tau));
                double c = 1.0 / std::sqrt(1.0 + t * t);
                double s = t * c;

                /* 更新B */
                B[p][p] = c * c * app - 2 * s * c * apq + s * s * aqq;
                B[q][q] = s * s * app + 2 * s * c * apq + c * c * aqq;
                B[p][q] = B[q][p] = 0.0;

                for (int r = 0; r < n; ++r) {
                    if (r == p || r == q) continue;
                    double brp = B[r][p], brq = B[r][q];
                    B[r][p] = B[p][r] = c * brp - s * brq;
                    B[r][q] = B[q][r] = s * brp + c * brq;
                }

                /* 更新V */
                for (int r = 0; r < n; ++r) {
                    double vrP = V[r][p], vrQ = V[r][q];
                    V[r][p] = c * vrP - s * vrQ;
                    V[r][q] = s * vrP + c * vrQ;
                }
            }
        }
    }

    /* 提取奇异值 */
    QVector<double> sigma(n);
    for (int i = 0; i < n; ++i) sigma[i] = std::sqrt(std::max(0.0, B[i][i]));

    /* 按降序排列 */
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(),
              [&sigma](int a, int b) { return sigma[a] > sigma[b]; });

    QVector<double> sortedSigma(n);
    QVector<QVector<double>> sortedV(n, QVector<double>(n));
    for (int i = 0; i < n; ++i) {
        sortedSigma[i] = sigma[order[i]];
        for (int j = 0; j < n; ++j) sortedV[j][i] = V[j][order[i]];
    }

    /* U = A * V * Sigma^-1 */
    int r = qMin(m, n);
    QVector<QVector<double>> U(m, QVector<double>(r, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < r; ++j) {
            double dot = 0.0;
            for (int k = 0; k < n; ++k) dot += A[i][k] * sortedV[k][j];
            U[i][j] = (sortedSigma[j] > 1e-15) ? dot / sortedSigma[j] : 0.0;
        }
    }

    SVDResult result;
    result.U = U;
    result.singularValues = sortedSigma;
    result.V = sortedV;

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    int rank = 0;
    for (double s : sortedSigma) if (s > 1e-10) rank++;
    emit decompositionCompleted(m, n, rank);

    return result;
}

QVector<QVector<double>> SingularValueDecomposition::lowRankApproximation(
    const QVector<QVector<double>>& A, int rank)
{
    auto svd = decompose(A);
    int m = A.size();
    int n = A.isEmpty() ? 0 : A[0].size();
    int r = qMin(rank, qMin(m, n));

    QVector<QVector<double>> result(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < r; ++k) {
                int ukCol = qMin(k, svd.U.isEmpty() ? 0 : svd.U[0].size() - 1);
                result[i][j] += svd.U[i][ukCol] * svd.singularValues[k] * svd.V[j][k];
            }
        }
    }
    return result;
}

QVector<QVector<double>> SingularValueDecomposition::pseudoInverse(
    const QVector<QVector<double>>& A)
{
    auto svd = decompose(A);
    int m = A.size();
    int n = A.isEmpty() ? 0 : A[0].size();
    double tol = 1e-10 * qMax(m, n) * (svd.singularValues.isEmpty() ? 1.0 : svd.singularValues[0]);

    /* Sigma+ */
    int r = svd.singularValues.size();
    QVector<QVector<double>> sigmaInv(r, QVector<double>(r, 0.0));
    for (int i = 0; i < r; ++i) {
        if (svd.singularValues[i] > tol)
            sigmaInv[i][i] = 1.0 / svd.singularValues[i];
    }

    /* A+ = V * Sigma+ * U^T */
    QVector<QVector<double>> result(n, QVector<double>(m, 0.0));
    int ur = svd.U.isEmpty() ? 0 : svd.U[0].size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            for (int k = 0; k < ur; ++k) {
                result[i][j] += svd.V[i][k] * sigmaInv[k][k] * svd.U[j][k];
            }
        }
    }
    return result;
}

double SingularValueDecomposition::conditionNumber(const QVector<QVector<double>>& A)
{
    auto svd = decompose(A);
    if (svd.singularValues.isEmpty()) return 0.0;
    double maxSv = svd.singularValues.first();
    double minSv = svd.singularValues.last();
    return (minSv > 1e-15) ? maxSv / minSv : 1e15;
}

void SingularValueDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
