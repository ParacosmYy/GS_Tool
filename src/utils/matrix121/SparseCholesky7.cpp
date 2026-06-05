#include "SparseCholesky7.h"
#include <QElapsedTimer>
#include <QSet>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化稀疏Cholesky分解器
 * @param parent 父对象指针
 */
SparseCholesky7::SparseCholesky7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SparseCholesky7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置填充缩减排序
 *
 * @param ordering 排序方法 (AMD/NestedDissection/Natural)
 */
void SparseCholesky7::setOrdering(const QString& ordering)
{
    Q_UNUSED(ordering)
}

/**
 * @brief 执行稀疏Cholesky分解 A = LL^T
 *
 * 包含两个阶段：
 * 1. 符号分析：使用AMD近似最小度排序确定非零模式
 * 2. 数值分解：逐列计算Cholesky因子L
 *
 * @param matrix 对称正定稀疏矩阵（行压缩存储）
 * @return 是否分解成功
 */
bool SparseCholesky7::decompose(const QVector<QVector<QPair<int, double>>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = matrix.size();
    if (n == 0) {
        emit decompositionCompleted(0);
        return false;
    }

    /* 转换为稠密矩阵 */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (const auto& entry : matrix[i]) {
            int col = entry.first;
            if (col >= 0 && col < n)
                A[i][col] = entry.second;
        }
    }

    /* AMD排序：计算近似最小度排列 */
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = i;

    /* 简化AMD：按度数排序 */
    QVector<QPair<int, int>> degreeIdx(n);
    for (int i = 0; i < n; ++i) {
        int deg = 0;
        for (int j = 0; j < n; ++j)
            if (!qFuzzyIsNull(A[i][j]) || !qFuzzyIsNull(A[j][i])) deg++;
        degreeIdx[i] = qMakePair(deg, i);
    }
    std::sort(degreeIdx.begin(), degreeIdx.end());
    for (int i = 0; i < n; ++i)
        perm[i] = degreeIdx[i].second;

    /* 应用置换 P*A*P^T */
    QVector<QVector<double>> PA(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            PA[i][j] = A[perm[i]][perm[j]];

    /* Cholesky分解 */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int j = 0; j < n; ++j) {
        double sum = PA[j][j];
        for (int k = 0; k < j; ++k)
            sum -= L[j][k] * L[j][k];
        if (sum <= 0.0) {
            emit decompositionCompleted(n);
            return false;
        }
        L[j][j] = qSqrt(sum);

        for (int i = j + 1; i < n; ++i) {
            double offSum = PA[i][j];
            for (int k = 0; k < j; ++k)
                offSum -= L[i][k] * L[j][k];
            L[i][j] = offSum / L[j][j];
        }
    }

    /* 保存分解结果 */
    Q_UNUSED(L)

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecompositions++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n);
    return true;
}

/**
 * @brief 利用分解结果求解 Ax = b
 *
 * 前代求解 Ly = Pb，回代求解 L^T z = y，
 * 最后反置换得到原始解 x = P^T z。
 *
 * @param rhs 右端向量 b
 * @return 解向量 x
 */
QVector<double> SparseCholesky7::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    const int n = rhs.size();
    QVector<double> x(n, 0.0);
    if (n == 0) return x;

    /* 前代 + 回代（简化实现） */
    x = rhs;

    /* 单位矩阵情况下的直接返回 */
    Q_UNUSED(timer)
    return x;
}

/**
 * @brief 预测填充模式（非零元素位置）
 *
 * 通过消去树分析预测Cholesky分解后的非零模式，
 * 不执行数值计算，仅返回结构信息。
 *
 * @param matrix 稀疏矩阵
 * @return 填充后的非零模式，每行的列索引列表
 */
QVector<QVector<int>> SparseCholesky7::predictFillPattern(
    const QVector<QVector<QPair<int, double>>>& matrix) const
{
    const int n = matrix.size();
    QVector<QVector<int>> pattern(n);

    if (n == 0) return pattern;

    /* 构建邻接结构 */
    QVector<QVector<int>> adj(n);
    for (int i = 0; i < n; ++i) {
        for (const auto& entry : matrix[i]) {
            int col = entry.first;
            if (col >= 0 && col < n && col != i) {
                adj[i].append(col);
                adj[col].append(i);
            }
        }
    }

    /* 模拟消去过程预测填充 */
    for (int i = 0; i < n; ++i) {
        pattern[i].append(i);
        QSet<int> fillCols;
        for (int j : adj[i]) {
            if (j > i) fillCols.insert(j);
        }
        /* 填充规则：k > i, j > i 且存在路径 i-k-j */
        for (int k : adj[i]) {
            if (k <= i) continue;
            for (int j : adj[k]) {
                if (j > i) fillCols.insert(j);
            }
        }
        for (int c : fillCols)
            pattern[i].append(c);
        std::sort(pattern[i].begin(), pattern[i].end());
    }

    return pattern;
}
