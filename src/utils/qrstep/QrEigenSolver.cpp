/**
 * @file QrEigenSolver.cpp
 * @brief QR算法特征值求解实现
 */

#include "utils/qrstep/QrEigenSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
QrEigenSolver::QrEigenSolver(QObject* parent)
    : QObject(parent)
{
}

/** @brief Wilkinson位移 */
double QrEigenSolver::wilkinsonShift(double a, double b, double c) const
{
    double delta = (a - c) / 2.0;
    double sign = (delta >= 0.0) ? 1.0 : -1.0;
    double disc = std::sqrt(delta * delta + b * b);
    if (disc < 1e-300) return c;
    return c - sign * b * b / (delta + sign * disc);
}

/** @brief 求解全部特征值 */
QVector<double> QrEigenSolver::solve(const QVector<QVector<double>>& A,
                                       int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0) return {};

    /* 复制矩阵并化为上Hessenberg */
    QVector<QVector<double>> H(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            H[i][j] = A[i][j];

    /* Householder化Hessenberg */
    for (int k = 0; k < n - 2; ++k) {
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i)
            norm += H[i][k] * H[i][k];
        norm = std::sqrt(norm);
        if (norm < 1e-15) continue;

        double s = (H[k + 1][k] >= 0.0) ? -norm : norm;
        double alpha = std::sqrt(2.0 / (norm * norm + std::abs(H[k + 1][k] * norm)));

        QVector<double> v(n, 0.0);
        v[k + 1] = (H[k + 1][k] - s) * alpha;
        for (int i = k + 2; i < n; ++i)
            v[i] = H[i][k] * alpha;

        /* H = (I - 2vv^T) H (I - 2vv^T) */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = k + 1; i < n; ++i)
                dot += v[i] * H[i][j];
            for (int i = k + 1; i < n; ++i)
                H[i][j] -= 2.0 * v[i] * dot;
        }
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j)
                dot += H[i][j] * v[j];
            for (int j = k + 1; j < n; ++j)
                H[i][j] -= 2.0 * dot * v[j];
        }
    }

    /* 带位移的QR迭代 */
    int totalIter = 0;
    int m = n - 1;

    while (m > 0 && totalIter < maxIter) {
        /* Wilkinson位移 */
        double shift = wilkinsonShift(H[m - 1][m - 1], H[m - 1][m], H[m][m]);

        /* 位移QR步 */
        for (int i = 0; i <= m; ++i)
            H[i][i] -= shift;

        /* Givens QR分解 */
        QVector<double> cosArr(m + 1), sinArr(m + 1);
        for (int i = 0; i < m; ++i) {
            double a = H[i][i], b = H[i + 1][i];
            double r = std::sqrt(a * a + b * b);
            if (r < 1e-300) { cosArr[i] = 1.0; sinArr[i] = 0.0; continue; }
            cosArr[i] = a / r;
            sinArr[i] = b / r;

            for (int j = i; j <= m; ++j) {
                double t1 = cosArr[i] * H[i][j] + sinArr[i] * H[i + 1][j];
                double t2 = -sinArr[i] * H[i][j] + cosArr[i] * H[i + 1][j];
                H[i][j] = t1;
                H[i + 1][j] = t2;
            }
        }

        /* RQ乘法 */
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j <= qMin(i + 2, m); ++j) {
                double t1 = cosArr[i] * H[j][i] + sinArr[i] * H[j][i + 1];
                double t2 = -sinArr[i] * H[j][i] + cosArr[i] * H[j][i + 1];
                H[j][i] = t1;
                H[j][i + 1] = t2;
            }
        }

        /* 恢复位移 */
        for (int i = 0; i <= m; ++i)
            H[i][i] += shift;

        totalIter++;

        /* 收敛检测 */
        if (std::abs(H[m][m - 1]) < 1e-10 * (std::abs(H[m - 1][m - 1]) + std::abs(H[m][m])))
            m--;
    }

    /* 提取特征值(对角线) */
    QVector<double> eigenvalues(n);
    for (int i = 0; i < n; ++i)
        eigenvalues[i] = H[i][i];

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, totalIter);
    return eigenvalues;
}

/** @brief 重置统计 */
void QrEigenSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
