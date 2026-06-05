/**
 * @file CirculantSolver.cpp
 * @brief 循环矩阵求解实现 — DFT对角化
 */

#include "utils/circulant/CirculantSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
CirculantSolver::CirculantSolver(QObject* parent)
    : QObject(parent)
{
}

/** @brief 内部DFT计算特征值(对角元素) */
QVector<double> CirculantSolver::dftDiag(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> result(n, 0.0);

    for (int k = 0; k < n; ++k) {
        double re = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = -2.0 * M_PI * k * j / n;
            re += input[j] * std::cos(angle);
        }
        result[k] = re;
    }
    return result;
}

/** @brief 求解循环系统 */
QVector<double> CirculantSolver::solve(const QVector<double>& firstColumn,
                                         const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = firstColumn.size();
    if (n == 0 || rhs.size() != n) return {};

    /* 计算循环矩阵的DFT特征值 */
    QVector<double> lambda = dftDiag(firstColumn);

    /* DFT变换rhs */
    QVector<double> rhsDft(n, 0.0);
    for (int k = 0; k < n; ++k) {
        double re = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = -2.0 * M_PI * k * j / n;
            re += rhs[j] * std::cos(angle);
        }
        rhsDft[k] = re;
    }

    /* 逐元素除法 */
    QVector<double> solDft(n, 0.0);
    for (int k = 0; k < n; ++k) {
        if (std::abs(lambda[k]) > 1e-300)
            solDft[k] = rhsDft[k] / lambda[k];
    }

    /* 逆DFT */
    QVector<double> x(n, 0.0);
    for (int j = 0; j < n; ++j) {
        double sum = 0.0;
        for (int k = 0; k < n; ++k) {
            double angle = 2.0 * M_PI * k * j / n;
            sum += solDft[k] * std::cos(angle);
        }
        x[j] = sum / n;
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n);
    return x;
}

/** @brief 重置统计 */
void CirculantSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
