/**
 * @file BandSolver2.cpp
 * @brief 带状矩阵求解器实现 — 带状矩阵分解 + 线性方程组求解
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现带状矩阵的 LU 分解和线性方程组求解。
 * 带状矩阵的非零元素集中在对角线附近的带状区域内，
 * 利用稀疏结构显著减少计算量和存储空间。
 */

#include "utils/matrix62/BandSolver2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化空的带状求解器
 * @param parent 父QObject对象
 */
BandSolver2::BandSolver2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BandSolver2"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置带宽参数
 *
 * @param lower 下带宽（主对角线以下的非零次对角线数）
 * @param upper 上带宽（主对角线以上的非零次对角线数）
 */
void BandSolver2::setBandwidth(int lower, int upper)
{
    m_lower = qMax(0, lower);
    m_upper = qMax(0, upper);
}

/**
 * @brief 设置矩阵维度
 * @param n 矩阵维度（n x n）
 */
void BandSolver2::setDimension(int n)
{
    m_n = qMax(0, n);
}

/**
 * @brief 设置带状矩阵数据
 *
 * bands 的存储格式：bands[i] 包含第 i 条对角线的元素。
 * - bands[0]: 主对角线（长度 n）
 * - bands[1]~bands[upper]: 上对角线（长度 n-1, n-2, ...）
 * - bands[upper+1]~bands[upper+lower]: 下对角线（长度 n-1, n-2, ...）
 *
 * @param bands 对角线存储的带状矩阵数据
 */
void BandSolver2::setMatrix(const QVector<QVector<double>>& bands)
{
    m_factor = bands;
    m_posDef = true;
}

// ──────────────────────────────────────────────
// 分解与求解
// ──────────────────────────────────────────────

/**
 * @brief 执行带状矩阵的 LU 分解（无主元选取版本）
 *
 * 利用带状结构的稀疏性，只对带内元素执行消元。
 * 分解后 L 和 U 存储在 m_factor 中（原地分解）。
 *
 * @return true 分解成功
 */
void BandSolver2::factorize()
{
    if (m_n == 0 || m_factor.isEmpty()) return;

    const int totalBands = m_factor.size();

    for (int k = 0; k < m_n; ++k) {
        // 主对角线元素
        double diag = m_factor[0][k];
        if (qAbs(diag) < 1e-15) {
            m_posDef = false;
            return;
        }

        // 消元：处理下对角线
        for (int i = 1; i <= m_lower && (k + i) < m_n; ++i) {
            int bandIdx = m_upper + i;
            if (bandIdx >= totalBands) break;
            if (k >= m_factor[bandIdx].size()) break;

            double factor = m_factor[bandIdx][k] / diag;
            m_factor[bandIdx][k] = factor; // 存储 L 的元素

            // 更新行
            for (int j = 1; j <= m_upper && (k + j) < m_n; ++j) {
                int bandJ = j;
                if (bandJ >= totalBands) break;
                int idx = k + j;
                if (idx < m_factor[bandJ].size()) {
                    m_factor[bandJ][idx] -= factor * m_factor[bandJ > 0 ? bandJ : 0][k];
                }
            }
        }

        // 检查正定性
        if (diag <= 0.0) {
            m_posDef = false;
        }
    }
}

/**
 * @brief 求解带状线性方程组 Ax = b
 *
 * 先执行 LU 分解，然后使用前代和回代求解。
 * 利用带状结构，每次只操作带内的非零元素。
 *
 * @param rhs 右端向量 b
 * @return 解向量 x
 */
QVector<double> BandSolver2::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || rhs.size() != m_n) return {};

    // 执行分解
    factorize();

    QVector<double> x = rhs;

    // 前代（解 Ly = b）
    for (int i = 0; i < m_n; ++i) {
        for (int j = qMax(0, i - m_lower); j < i; ++j) {
            int bandIdx = m_upper + (i - j);
            if (bandIdx < m_factor.size() && j < m_factor[bandIdx].size()) {
                x[i] -= m_factor[bandIdx][j] * x[j];
            }
        }
    }

    // 回代（解 Ux = y）
    for (int i = m_n - 1; i >= 0; --i) {
        for (int j = i + 1; j <= qMin(m_n - 1, i + m_upper); ++j) {
            int bandIdx = j - i;
            if (bandIdx < m_factor.size() && i < m_factor[bandIdx].size()) {
                x[i] -= m_factor[bandIdx][i] * x[j];
            }
        }
        if (i < m_factor[0].size() && qAbs(m_factor[0][i]) > 1e-15) {
            x[i] /= m_factor[0][i];
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalSystems += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    // 计算残差近似
    double residual = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double axi = 0.0;
        if (i < m_factor[0].size()) axi += m_factor[0][i] * x[i];
        residual = qMax(residual, qAbs(rhs[i] - axi));
    }

    emit solveCompleted(m_n, residual);
    return x;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含求解次数、总系统规模和平均耗时的Stats结构
 */
BandSolver2::Stats BandSolver2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void BandSolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
