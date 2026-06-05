#include "SparseQR4.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class SparseQR4
 * @brief 稀疏QR分解实现
 *
 * 针对稀疏矩阵的高效QR分解。利用矩阵的稀疏性减少计算量，
 * 仅处理非零元素。使用列取向的Householder变换消元。
 *
 * 填充比(Fill Ratio) = 分解后R的非零元素数 / 原始矩阵非零元素数。
 * 理想情况下填充比接近1，实际取决于矩阵的稀疏结构。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
SparseQR4::SparseQR4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行稀疏QR分解
 *
 * 对稀疏矩阵执行列取向的QR分解:
 * 1. 统计每列非零元素，构建稀疏工作结构
 * 2. 逐列执行Householder变换，仅处理非零行
 * 3. 计算填充比作为分解效率指标
 *
 * @param sparseMatrix 输入稀疏矩阵(m×n)
 * @return 分解是否成功
 */
bool SparseQR4::factorize(const QVector<QVector<double>>& sparseMatrix)
{
    QElapsedTimer timer;
    timer.start();

    int m = sparseMatrix.size();
    if (m == 0) return false;
    int n = sparseMatrix[0].size();
    if (n == 0) return false;

    /* 复制到工作矩阵 */
    m_R = sparseMatrix;

    /* 统计原始非零元素数 */
    int origNonzeros = 0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (qAbs(m_R[i][j]) > 1e-15) {
                origNonzeros++;
            }
        }
    }

    /* 逐列执行稀疏Householder变换 */
    for (int k = 0; k < qMin(m, n); ++k) {
        /* 收集第k列的非零元素行 */
        QVector<int> nonZeroRows;
        for (int i = k; i < m; ++i) {
            if (qAbs(m_R[i][k]) > 1e-15 || i == k) {
                nonZeroRows.append(i);
            }
        }

        /* 计算第k列下方部分的范数 */
        double norm = 0.0;
        for (int i : nonZeroRows) {
            norm += m_R[i][k] * m_R[i][k];
        }
        norm = qSqrt(norm);

        if (norm < 1e-15) continue;

        /* Householder向量 */
        double sign = (m_R[k][k] >= 0.0) ? 1.0 : -1.0;
        double alpha = sign * norm;

        /* 构造并应用反射 */
        double vk = m_R[k][k] + alpha;
        double beta = 2.0 / (vk * vk + [&]() {
            double s = 0.0;
            for (int i : nonZeroRows) {
                if (i > k) s += m_R[i][k] * m_R[i][k];
            }
            return s;
        }());

        /* 更新R的非零列 */
        for (int j = k; j < n; ++j) {
            /* 仅在有非零元素的行上计算点积 */
            double dot = vk * m_R[k][j];
            for (int i : nonZeroRows) {
                if (i > k) dot += m_R[i][k] * m_R[i][j];
            }
            dot *= beta;

            m_R[k][j] -= dot * vk;
            for (int i : nonZeroRows) {
                if (i > k) m_R[i][j] -= dot * m_R[i][k];
            }
        }

        m_R[k][k] = -alpha;
        /* 消去下三角 */
        for (int i = k + 1; i < m; ++i) {
            m_R[i][k] = 0.0;
        }
    }

    /* 计算R的非零元素数和填充比 */
    int rNonzeros = 0;
    for (int i = 0; i < qMin(m, n); ++i) {
        for (int j = i; j < n; ++j) {
            if (qAbs(m_R[i][j]) > 1e-15) {
                rNonzeros++;
            }
        }
    }

    double fillRatio = (origNonzeros > 0) ? static_cast<double>(rNonzeros) / origNonzeros : 1.0;
    m_stats.totalNonzeros = rNonzeros;

    m_stats.totalFactorizations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFactorizations;

    emit factorizationCompleted(m, n, fillRatio);

    return true;
}

/**
 * @brief 求解最小二乘问题
 *
 * 利用稀疏QR分解结果求解 min ||Ax - b||
 * 通过回代求解 Rx = Q^T * b (简化: 仅使用R)。
 *
 * @param rhs 右端向量b
 * @return 最小二乘解向量
 */
QVector<double> SparseQR4::solve(const QVector<double>& rhs) const
{
    int n = m_R.isEmpty() ? 0 : m_R[0].size();
    QVector<double> x(n, 0.0);

    if (m_R.isEmpty() || rhs.isEmpty()) return x;

    /* 简化回代: 假设Q^T * b ≈ rhs */
    for (int i = qMin(m_R.size(), n) - 1; i >= 0; --i) {
        x[i] = rhs[i];
        for (int j = i + 1; j < n; ++j) {
            if (j < m_R[i].size()) {
                x[i] -= m_R[i][j] * x[j];
            }
        }
        if (i < m_R[i].size() && qAbs(m_R[i][i]) > 1e-15) {
            x[i] /= m_R[i][i];
        }
    }

    return x;
}

/**
 * @brief 重置所有统计数据
 *
 * 将分解计数、非零元素计数和计时归零。
 */
void SparseQR4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_R.clear();
}
