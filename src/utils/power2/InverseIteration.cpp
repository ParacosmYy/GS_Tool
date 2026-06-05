/**
 * @file InverseIteration.cpp
 * @brief 反幂迭代实现 — 求解最接近平移的特征对
 *
 * 迭代格式: (A - sigma*I) * y = x_{k}
 *           x_{k+1} = y / ||y||
 *           mu_{k} = x_{k+1}^T * A * x_{k+1} (Rayleigh商)
 * 收敛后mu即为最接近sigma的特征值，x为对应特征向量。
 */

#include "utils/power2/InverseIteration.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

InverseIteration::InverseIteration(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> InverseIteration::solveLinearSystem(QVector<QVector<double>> A,
                                                    QVector<double> b)
{
    int n = A.size();
    if (n == 0) return {};

    /* 部分主元Gauss消元 */
    for (int k = 0; k < n; ++k) {
        /* 寻找主元 */
        int pivot = k;
        double maxVal = std::abs(A[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (std::abs(A[i][k]) > maxVal) {
                maxVal = std::abs(A[i][k]);
                pivot = i;
            }
        }

        if (maxVal < 1e-15) return {}; /* 奇异矩阵 */

        /* 交换行 */
        if (pivot != k) {
            std::swap(A[k], A[pivot]);
            std::swap(b[k], b[pivot]);
        }

        /* 消元 */
        for (int i = k + 1; i < n; ++i) {
            double factor = A[i][k] / A[k][k];
            for (int j = k + 1; j < n; ++j)
                A[i][j] -= factor * A[k][j];
            b[i] -= factor * b[k];
            A[i][k] = 0.0;
        }
    }

    /* 回代 */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = b[i];
        for (int j = i + 1; j < n; ++j)
            sum -= A[i][j] * x[j];
        if (std::abs(A[i][i]) < 1e-15) return {};
        x[i] = sum / A[i][i];
    }
    return x;
}

QPair<double, QVector<double>>
InverseIteration::solve(const QVector<QVector<double>>& A,
                        double shift, double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0) return {0.0, {}};

    /* 构造 (A - shift * I) */
    QVector<QVector<double>> As(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            As[i][j] = A[i][j] - ((i == j) ? shift : 0.0);

    /* 随机初始向量 */
    std::mt19937 gen(12345);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    QVector<double> x(n);
    for (int i = 0; i < n; ++i) x[i] = dist(gen);

    /* 归一化初始向量 */
    double nrm = 0.0;
    for (double v : x) nrm += v * v;
    nrm = std::sqrt(nrm);
    for (int i = 0; i < n; ++i) x[i] /= nrm;

    double eigenvalue = shift;
    int iterations = 0;

    for (int iter = 0; iter < maxIter; ++iter) {
        iterations = iter + 1;

        /* 求解 (A - shift*I) * y = x */
        QVector<double> y = solveLinearSystem(As, x);
        if (y.isEmpty()) break;

        /* 归一化y */
        double yNorm = 0.0;
        for (double v : y) yNorm += v * v;
        yNorm = std::sqrt(yNorm);
        if (yNorm < 1e-15) break;
        for (int i = 0; i < n; ++i) y[i] /= yNorm;

        /* Rayleigh商: eigenvalue = y^T * A * y */
        double mu = 0.0;
        for (int i = 0; i < n; ++i) {
            double aiy = 0.0;
            for (int j = 0; j < n; ++j)
                aiy += A[i][j] * y[j];
            mu += y[i] * aiy;
        }

        /* 收敛判断: |mu - eigenvalue| < tol */
        if (iter > 0 && std::abs(mu - eigenvalue) < tol) {
            eigenvalue = mu;
            x = y;
            break;
        }

        eigenvalue = mu;
        x = y;
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(eigenvalue, iterations);
    return {eigenvalue, x};
}

void InverseIteration::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
