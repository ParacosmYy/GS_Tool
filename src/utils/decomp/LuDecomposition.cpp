/**
 * @file LuDecomposition.cpp
 * @brief LU分解实现 — Doolittle+部分主元
 */

#include "utils/decomp/LuDecomposition.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
LuDecomposition::LuDecomposition(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief LU分解(LUP) */
LuDecomposition::LuResult LuDecomposition::decompose(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    LuResult result;
    int n = matrix.size();
    if (n == 0) return result;

    /* 拷贝到U，初始化L和置换向量 */
    result.U = matrix;
    result.L = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    result.permutation.resize(n);
    for (int i = 0; i < n; ++i) result.permutation[i] = i;

    /* 部分主元选取的Doolittle分解 */
    for (int col = 0; col < n; ++col) {
        /* 找主元 */
        int maxRow = col;
        double maxVal = qAbs(result.U[col][col]);
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(result.U[row][col]) > maxVal) {
                maxVal = qAbs(result.U[row][col]);
                maxRow = row;
            }
        }

        /* 交换行 */
        if (maxRow != col) {
            std::swap(result.U[col], result.U[maxRow]);
            std::swap(result.permutation[col], result.permutation[maxRow]);
            /* 交换L已计算部分 */
            for (int k = 0; k < col; ++k)
                std::swap(result.L[col][k], result.L[maxRow][k]);
        }

        if (qAbs(result.U[col][col]) < 1e-15) {
            result.success = false;
            result.determinant = 0.0;
            return result;
        }

        result.L[col][col] = 1.0;

        for (int row = col + 1; row < n; ++row) {
            double factor = result.U[row][col] / result.U[col][col];
            result.L[row][col] = factor;
            for (int j = col; j < n; ++j)
                result.U[row][j] -= factor * result.U[col][j];
        }
    }

    /* 计算行列式 det(A) = (-1)^s * prod(U_ii) */
    result.determinant = 1.0;
    int swaps = 0;
    for (int i = 0; i < n; ++i) {
        result.determinant *= result.U[i][i];
        if (result.permutation[i] != i) ++swaps;
    }
    /* 计算置换的奇偶性 */
    QVector<bool> visited(n, false);
    int cycles = 0;
    for (int i = 0; i < n; ++i) {
        if (!visited[i]) {
            int j = i;
            while (!visited[j]) {
                visited[j] = true;
                j = result.permutation[j];
            }
            ++cycles;
        }
    }
    if ((n - cycles) % 2 != 0) result.determinant = -result.determinant;

    result.success = true;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalDecompositions;
    double total = static_cast<double>(m_stats.totalDecompositions + m_stats.totalSolves);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decompositionCompleted(n, result.determinant);
    return result;
}

/** @brief 利用LU分解求解Ax=b */
QVector<double> LuDecomposition::solve(const LuResult& lu,
                                        const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = lu.L.size();
    if (!lu.success || n == 0) return QVector<double>();

    /* 应用置换: Pb */
    QVector<double> pb(n);
    for (int i = 0; i < n; ++i)
        pb[i] = b[lu.permutation[i]];

    /* 前代: Ly = Pb */
    QVector<double> y(n);
    for (int i = 0; i < n; ++i) {
        y[i] = pb[i];
        for (int j = 0; j < i; ++j)
            y[i] -= lu.L[i][j] * y[j];
    }

    /* 回代: Ux = y */
    QVector<double> x(n);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= lu.U[i][j] * x[j];
        x[i] /= lu.U[i][i];
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalSolves;
    double total = static_cast<double>(m_stats.totalDecompositions + m_stats.totalSolves);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit solveCompleted(n);
    return x;
}

/** @brief 计算行列式 */
double LuDecomposition::determinant(const QVector<QVector<double>>& matrix)
{
    auto result = decompose(matrix);
    return result.determinant;
}

/** @brief 矩阵求逆 */
QVector<QVector<double>> LuDecomposition::inverse(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return QVector<QVector<double>>();

    auto lu = decompose(matrix);
    if (!lu.success) return QVector<QVector<double>>();

    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    for (int col = 0; col < n; ++col) {
        QVector<double> e(n, 0.0);
        e[col] = 1.0;
        QVector<double> x = solve(lu, e);
        for (int row = 0; row < n; ++row)
            inv[row][col] = x[row];
    }

    emit solveCompleted(n);
    return inv;
}

/** @brief 重置统计 */
void LuDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
