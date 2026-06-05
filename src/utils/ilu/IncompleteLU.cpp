/**
 * @file IncompleteLU.cpp
 * @brief 不完全LU分解(ILU(0))实现
 */

#include "utils/ilu/IncompleteLU.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
IncompleteLU::IncompleteLU(QObject* parent)
    : QObject(parent)
{
}

/** @brief ILU(0)分解 */
bool IncompleteLU::factorize(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0) return false;

    m_LU = A;

    for (int i = 1; i < n; ++i) {
        for (int k = 0; k < i; ++k) {
            if (k >= m_LU[i].size() || k >= m_LU[k].size()) continue;
            if (std::abs(m_LU[k][k]) < 1e-300) continue;
            if (std::abs(m_LU[i][k]) < 1e-300) continue;

            m_LU[i][k] /= m_LU[k][k];

            for (int j = k + 1; j < n; ++j) {
                if (std::abs(m_LU[i][j]) > 1e-300 && std::abs(m_LU[k][j]) > 1e-300)
                    m_LU[i][j] -= m_LU[i][k] * m_LU[k][j];
            }
        }
    }

    m_stats.totalFactorizations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalFactorizations + m_stats.totalSolves);

    emit factorizationCompleted(n);
    return true;
}

/** @brief 求解LUx = b */
QVector<double> IncompleteLU::solve(const QVector<double>& rhs) const
{
    int n = m_LU.size();
    if (n == 0 || rhs.size() != n) return {};

    /* 前代: Ly = b */
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        y[i] = rhs[i];
        for (int j = 0; j < i; ++j)
            y[i] -= m_LU[i][j] * y[j];
    }

    /* 回代: Ux = y */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= m_LU[i][j] * x[j];
        if (std::abs(m_LU[i][i]) > 1e-300)
            x[i] /= m_LU[i][i];
    }
    return x;
}

/** @brief 重置统计 */
void IncompleteLU::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
