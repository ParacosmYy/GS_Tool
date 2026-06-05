/**
 * @file BandMatrixSolver.cpp
 * @brief 带状线性方程组求解实现
 */

#include "utils/bandmat/BandMatrixSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
BandMatrixSolver::BandMatrixSolver(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 带状 LU 分解 + 前代回代
 *
 * 带状存储格式: bandMatrix[i][j] 存储第 i 行中
 * 列号从 (i - bandwidth) 到 (i + bandwidth) 的元素，
 * j 索引偏移后: 实际列 = i - bandwidth + j。
 * 当实际列 < 0 或 >= n 时该位置视为零。
 *
 * LU 分解仅操作带内元素，复杂度 O(n·m²)。
 */
QVector<double> BandMatrixSolver::solve(
    QVector<QVector<double>> bandMatrix,
    QVector<double> rhs, int bandwidth)
{
    QElapsedTimer timer;
    timer.start();

    const int n = static_cast<int>(rhs.size());
    const int bandWidth = bandwidth;
    const int bandCols = 2 * bandWidth + 1;

    /* 工作副本 */
    QVector<QVector<double>> B(n, QVector<double>(bandCols, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < bandCols; ++j) {
            if (j < bandMatrix[i].size())
                B[i][j] = bandMatrix[i][j];
        }
    }
    QVector<double> d(rhs);

    /** @brief 将带状索引 (i, j) 映射到实际列号 */
    auto actualCol = [bandWidth](int i, int j) -> int {
        return i - bandWidth + j;
    };

    /** @brief 获取 B[i][k] 对应实际列 col 的带内索引 */
    auto bandIndex = [bandWidth](int row, int col) -> int {
        return col - row + bandWidth;
    };

    /* ---- LU 分解(带内消元) ---- */
    for (int k = 0; k < n; ++k) {
        /* 对角元素 */
        int jDiag = bandIndex(k, k);
        if (jDiag < 0 || jDiag >= bandCols) continue;
        double pivot = B[k][jDiag];
        if (std::abs(pivot) < 1e-300) continue;

        /* 消去第 k 列下方 bandWidth 行 */
        for (int i = k + 1; i < std::min(k + bandWidth + 1, n); ++i) {
            int jIk = bandIndex(i, k);
            if (jIk < 0 || jIk >= bandCols) continue;

            double factor = B[i][jIk] / pivot;
            B[i][jIk] = factor;  /* 存储 L 因子 */

            /* 更新第 i 行的带内元素 */
            for (int col = k + 1; col < std::min(i + bandWidth + 1, n); ++col) {
                int jIcol = bandIndex(i, col);
                int jKcol = bandIndex(k, col);
                if (jIcol >= 0 && jIcol < bandCols &&
                    jKcol >= 0 && jKcol < bandCols) {
                    B[i][jIcol] -= factor * B[k][jKcol];
                }
            }

            /* 更新右端项 */
            d[i] -= factor * d[k];
        }
    }

    /* ---- 回代 ---- */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = d[i];
        for (int col = i + 1; col < std::min(i + bandWidth + 1, n); ++col) {
            int jIcol = bandIndex(i, col);
            if (jIcol >= 0 && jIcol < bandCols)
                sum -= B[i][jIcol] * x[col];
        }
        int jDiag = bandIndex(i, i);
        if (jDiag >= 0 && jDiag < bandCols &&
            std::abs(B[i][jDiag]) > 1e-300)
            x[i] = sum / B[i][jDiag];
        else
            x[i] = 0.0;
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(n);
    return x;
}

/** @brief 重置统计 */
void BandMatrixSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
