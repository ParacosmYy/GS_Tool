/**
 * @file CholeskyUpdate.cpp
 * @brief Cholesky秩-1更新/降级实现
 *
 * 更新: L'满足L'*L'^T = L*L^T + x*x^T，使用一系列Givens旋转
 *       将[L^T; x^T]恢复为上三角形式。
 * 降级: L'满足L'*L'^T = L*L^T - x*x^T，使用双曲旋转或
 *       等价的Givens方法，需保证结果正定。
 */

#include "utils/cholupdate/CholeskyUpdate.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

CholeskyUpdate::CholeskyUpdate(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

bool CholeskyUpdate::update(QVector<QVector<double>>& L,
                            const QVector<double>& x)
{
    QElapsedTimer timer;
    timer.start();

    int n = L.size();
    if (n == 0 || static_cast<int>(x.size()) != n) return false;

    /* 将L^T扩展为增广上三角 [L^T; x^T] */
    QVector<QVector<double>> M(n + 1, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i; j < n; ++j)
            M[i][j] = L[j][i];
    for (int j = 0; j < n; ++j)
        M[n][j] = x[j];

    /* 逐列用Givens旋转消除底部行元素，恢复上三角 */
    for (int j = 0; j < n; ++j) {
        int pivotRow = j;
        int elimRow  = n;

        double a = M[pivotRow][j];
        double b = M[elimRow][j];
        if (std::abs(b) < 1e-15) continue;

        double r = std::sqrt(a * a + b * b);
        double c = a / r;
        double s = b / r;

        for (int k = j; k < n; ++k) {
            double m1 = M[pivotRow][k], m2 = M[elimRow][k];
            M[pivotRow][k] = c * m1 + s * m2;
            M[elimRow][k]  = -s * m1 + c * m2;
        }
        M[elimRow][j] = 0.0;
    }

    /* 提取更新后的L (下三角) */
    for (int i = 0; i < n; ++i)
        for (int j = 0; j <= i; ++j)
            L[i][j] = M[j][i];

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit updateCompleted(n);
    return true;
}

bool CholeskyUpdate::downdate(QVector<QVector<double>>& L,
                              const QVector<double>& x)
{
    QElapsedTimer timer;
    timer.start();

    int n = L.size();
    if (n == 0 || static_cast<int>(x.size()) != n) return false;

    /* 求解 L * z = x => z = L^{-1} * x */
    QVector<double> z(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = x[i];
        for (int j = 0; j < i; ++j)
            sum -= L[i][j] * z[j];
        if (std::abs(L[i][i]) < 1e-15) return false;
        z[i] = sum / L[i][i];
    }

    /* 检查正定性: ||z||^2 <= 1 */
    double zSq = 0.0;
    for (double val : z) zSq += val * val;
    if (zSq > 1.0) return false;

    /* alpha = sqrt(1 - ||z||^2) */
    double alpha = std::sqrt(std::max(0.0, 1.0 - zSq));

    /* 构造增广矩阵 [L^T; 0 ... alpha] 并应用旋转消除z分量 */
    QVector<double> row(n, 0.0);
    for (int i = 0; i < n; ++i)
        row[i] = z[i];

    for (int j = 0; j < n; ++j) {
        double a = L[j][j];
        double b = row[j];
        if (std::abs(b) < 1e-15) continue;

        double denom = std::sqrt(a * a - b * b);
        if (denom < 1e-15) return false;

        double c = a / denom;
        double s = -b / denom;

        /* 更新L的第j行 */
        L[j][j] = denom;
        for (int k = j + 1; k < n; ++k) {
            double lk = L[k][j];
            double rk = row[k];
            L[k][j] = c * lk - s * rk;
            row[k]   = -s * lk + c * rk;
        }
        row[j] = 0.0;
    }

    /* 确保对角线非负 */
    for (int i = 0; i < n; ++i) {
        if (L[i][i] < 0) {
            L[i][i] = -L[i][i];
            for (int k = i + 1; k < n; ++k)
                L[k][i] = -L[k][i];
        }
    }

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit updateCompleted(n);
    return true;
}

void CholeskyUpdate::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
