/**
 * @file HankelSolver.cpp
 * @brief Hankel矩阵求解实现 — LU分解
 */

#include "utils/hankel/HankelSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
HankelSolver::HankelSolver(QObject* parent)
    : QObject(parent)
{
}

/** @brief 求解Hankel系统 */
QVector<double> HankelSolver::solve(const QVector<double>& moments,
                                      const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = rhs.size();
    if (n == 0 || moments.size() < 2 * n - 1) return {};

    /* 构建Hankel矩阵: H[i][j] = moments[i+j] */
    QVector<QVector<double>> H(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            H[i][j] = moments[i + j];

    /* 带部分主元选择的LU分解 */
    QVector<int> pivot(n);
    for (int i = 0; i < n; ++i) pivot[i] = i;

    for (int k = 0; k < n; ++k) {
        double maxVal = std::abs(H[k][k]);
        int maxRow = k;
        for (int i = k + 1; i < n; ++i) {
            if (std::abs(H[i][k]) > maxVal) {
                maxVal = std::abs(H[i][k]);
                maxRow = i;
            }
        }
        if (maxRow != k) {
            std::swap(H[k], H[maxRow]);
            std::swap(pivot[k], pivot[maxRow]);
        }
        if (std::abs(H[k][k]) < 1e-15) continue;

        for (int i = k + 1; i < n; ++i) {
            H[i][k] /= H[k][k];
            for (int j = k + 1; j < n; ++j)
                H[i][j] -= H[i][k] * H[k][j];
        }
    }

    /* 解Ly = Pb */
    QVector<double> b(n);
    for (int i = 0; i < n; ++i) b[i] = rhs[pivot[i]];

    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        y[i] = b[i];
        for (int j = 0; j < i; ++j)
            y[i] -= H[i][j] * y[j];
    }

    /* 解Ux = y */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= H[i][j] * x[j];
        if (std::abs(H[i][i]) > 1e-300)
            x[i] /= H[i][i];
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n);
    return x;
}

/** @brief 重置统计 */
void HankelSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
