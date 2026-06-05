/**
 * @file SparseLU2.cpp
 * @brief 稀疏LU分解实现 — COO格式稀疏矩阵的LU分解与求解
 *
 * 实现稀疏矩阵的部分主元LU分解:
 * - 从COO(Coordinate)格式构建稀疏矩阵
 * - 部分主元(partial pivoting) LU分解: PA = LU
 * - 基于分解结果求解线性方程组 Ax = b
 * - 支持矩阵秩计算
 * - 统计分解次数、求解次数、平均耗时
 *
 * 算法:
 * 1. COO → 稠密存储(适合中小规模矩阵)
 * 2. 部分主元高斯消元: 每列选择最大主元，行交换
 * 3. L存储乘子，U存储消元结果，P存储置换
 * 4. 求解: Ly = Pb → Ux = y (前代+回代)
 */

#include "utils/matrix41/SparseLU2.h"

#include <QElapsedTimer>
#include <QtGlobal>

#include <cmath>
#include <algorithm>

/* ===== 公有方法实现 ===== */

/**
 * @brief 构造函数 — 初始化稀疏LU分解器
 * @param parent QObject父对象
 */
SparseLU2::SparseLU2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 从COO格式构建稀疏矩阵
 *
 * COO格式: 三个等长数组分别存储行索引、列索引、非零值。
 * 重复位置的值会被累加。
 *
 * @param rows 行索引数组
 * @param cols 列索引数组
 * @param vals 非零值数组
 * @param n 矩阵维度(n x n)
 */
void SparseLU2::buildFromCOO(const QVector<int>& rows,
                               const QVector<int>& cols,
                               const QVector<double>& vals,
                               int n)
{
    m_n = qMax(0, n);
    m_factored = false;

    if (m_n == 0) {
        m_LU.clear();
        m_pivot.clear();
        return;
    }

    /* 初始化稠密矩阵为零 */
    m_LU.resize(m_n * m_n, 0.0);

    /* 填充数据 — COO转稠密 */
    int nnz = qMin(qMin(rows.size(), cols.size()), vals.size());
    for (int i = 0; i < nnz; ++i) {
        int r = rows[i];
        int c = cols[i];
        if (r >= 0 && r < m_n && c >= 0 && c < m_n) {
            m_LU[r * m_n + c] += vals[i];
        }
    }

    /* 初始化置换向量为恒等置换 */
    m_pivot.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_pivot[i] = i;
    }
}

/**
 * @brief 执行LU分解 — 部分主元高斯消元
 *
 * 算法流程:
 * 对每一列k (0 ≤ k < n):
 * 1. 在第k列下方找绝对值最大的元素作为主元
 * 2. 交换行，记录置换
 * 3. 计算乘子 L[i][k] = LU[i][k] / LU[k][k]
 * 4. 消元: LU[i][j] -= L[i][k] * LU[k][j]
 *
 * @return 分解是否成功(失败原因: 奇异矩阵或未初始化)
 */
bool SparseLU2::decompose()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 0 || m_LU.isEmpty()) {
        m_factored = false;
        m_stats.totalDecompositions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            (m_stats.totalDecompositions + m_stats.totalSolves > 0)
                ? m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves)
                : 0.0;
        emit decompositionComplete(m_n, false);
        return false;
    }

    /* 初始化置换向量 */
    m_pivot.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_pivot[i] = i;
    }

    bool success = true;

    for (int k = 0; k < m_n; ++k) {
        /* 步骤1: 部分主元选取 — 找第k列下方绝对值最大的元素 */
        int maxRow = k;
        double maxVal = std::abs(m_LU[k * m_n + k]);
        for (int i = k + 1; i < m_n; ++i) {
            double val = std::abs(m_LU[i * m_n + k]);
            if (val > maxVal) {
                maxVal = val;
                maxRow = i;
            }
        }

        /* 检查奇异性 */
        if (maxVal < 1e-14) {
            success = false;
            continue;  /* 跳过零主元列，继续分解 */
        }

        /* 步骤2: 行交换 */
        if (maxRow != k) {
            for (int j = 0; j < m_n; ++j) {
                std::swap(m_LU[k * m_n + j], m_LU[maxRow * m_n + j]);
            }
            std::swap(m_pivot[k], m_pivot[maxRow]);
        }

        /* 步骤3: 消元 */
        double pivot = m_LU[k * m_n + k];
        for (int i = k + 1; i < m_n; ++i) {
            double multiplier = m_LU[i * m_n + k] / pivot;
            m_LU[i * m_n + k] = multiplier;  /* 存储L的乘子 */

            /* 更新U部分 */
            for (int j = k + 1; j < m_n; ++j) {
                m_LU[i * m_n + j] -= multiplier * m_LU[k * m_n + j];
            }
        }
    }

    m_factored = success;

    /* 更新统计信息 */
    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalDecompositions + m_stats.totalSolves > 0)
            ? m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves)
            : 0.0;

    emit decompositionComplete(m_n, success);
    return success;
}

/**
 * @brief 求解线性方程组 Ax = b
 *
 * 前提: 必须先调用decompose()成功分解
 * 求解步骤:
 * 1. 应用置换: Pb
 * 2. 前代: Ly = Pb (L为单位下三角)
 * 3. 回代: Ux = y (U为上三角)
 *
 * @param rhs 右端向量b
 * @return 解向量x，空向量表示求解失败
 */
QVector<double> SparseLU2::solve(const QVector<double>& rhs) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;

    if (!m_factored || rhs.size() != m_n || m_n <= 0) {
        const_cast<SparseLU2*>(this)->m_stats.totalSolves++;
        const_cast<SparseLU2*>(this)->m_timeSum += timer.elapsed();
        const_cast<SparseLU2*>(this)->m_stats.avgProcessingTimeMs =
            (m_stats.totalDecompositions + m_stats.totalSolves > 0)
                ? m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves)
                : 0.0;
        return result;
    }

    /* 步骤1: 应用置换 Pb */
    QVector<double> b(m_n);
    for (int i = 0; i < m_n; ++i) {
        b[i] = rhs[m_pivot[i]];
    }

    /* 步骤2: 前代 Ly = Pb (L为单位下三角，乘子存储在LU下三角) */
    QVector<double> y = b;
    for (int i = 1; i < m_n; ++i) {
        for (int j = 0; j < i; ++j) {
            y[i] -= m_LU[i * m_n + j] * y[j];
        }
    }

    /* 步骤3: 回代 Ux = y */
    QVector<double> x = y;
    for (int i = m_n - 1; i >= 0; --i) {
        for (int j = i + 1; j < m_n; ++j) {
            x[i] -= m_LU[i * m_n + j] * x[j];
        }
        double diag = m_LU[i * m_n + i];
        if (std::abs(diag) < 1e-14) {
            x[i] = 0.0;  /* 奇异矩阵，返回0 */
        } else {
            x[i] /= diag;
        }
    }

    result = x;

    /* 更新统计信息 */
    const_cast<SparseLU2*>(this)->m_stats.totalSolves++;
    const_cast<SparseLU2*>(this)->m_timeSum += timer.elapsed();
    const_cast<SparseLU2*>(this)->m_stats.avgProcessingTimeMs =
        (m_stats.totalDecompositions + m_stats.totalSolves > 0)
            ? m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves)
            : 0.0;

    return result;
}

/**
 * @brief 计算矩阵的秩
 *
 * 秩 = U中对角线元素绝对值 > 阈值的行数
 * 必须在decompose()之后调用。
 *
 * @return 矩阵秩，0表示未分解
 */
int SparseLU2::rank() const
{
    if (!m_factored || m_n <= 0) return 0;

    int r = 0;
    for (int i = 0; i < m_n; ++i) {
        if (std::abs(m_LU[i * m_n + i]) > 1e-10) {
            r++;
        }
    }
    return r;
}

/**
 * @brief 重置统计信息
 */
void SparseLU2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
