/**
 * @file SparseGMRES2.cpp
 * @brief Householder QR分解实现
 *
 * 实现基于Householder反射的QR分解，支持线性方程组求解
 * 和残差范数计算。适用于最小二乘问题和矩阵计算。
 */

#include "utils/matrix77/SparseGMRES2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
SparseGMRES2::SparseGMRES2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置待分解矩阵
 * @param A 输入矩阵，m行n列
 */
void SparseGMRES2::setMatrix(const QVector<QVector<double>>& A)
{
    if (A.isEmpty()) return;
    m_rows = A.size();
    m_cols = A[0].size();

    /* 拷贝到内部存储R矩阵 */
    m_R.clear();
    m_R.resize(m_rows);
    for (int i = 0; i < m_rows; ++i) {
        m_R[i].resize(m_cols, 0.0);
        for (int j = 0; j < qMin(static_cast<int>(A[i].size()), m_cols); ++j) {
            m_R[i][j] = A[i][j];
        }
    }
    m_Q.clear();
}

/**
 * @brief 执行QR分解
 * @return 分解是否成功
 *
 * 使用Householder反射逐列消元，将A分解为正交矩阵Q和上三角矩阵R。
 * Householder向量v通过列向量的范数计算得到，反射矩阵H = I - 2vv^T/v^Tv。
 */
bool SparseGMRES2::decompose()
{
    QElapsedTimer timer;
    timer.start();

    if (m_rows == 0 || m_cols == 0 || m_R.isEmpty()) return false;

    int m = m_rows;
    int n = m_cols;
    int minDim = qMin(m, n);

    /* 初始化Q为单位矩阵 */
    m_Q.resize(m);
    for (int i = 0; i < m; ++i) {
        m_Q[i].resize(m, 0.0);
        m_Q[i][i] = 1.0;
    }

    /* Householder变换逐列消元 */
    for (int k = 0; k < minDim; ++k) {
        /* 计算第k列下半部分的二范数 */
        double norm = 0.0;
        for (int i = k; i < m; ++i) {
            norm += m_R[i][k] * m_R[i][k];
        }
        norm = qSqrt(norm);

        if (norm < 1e-15) continue;

        /* 计算Householder向量参数 */
        double alpha = (m_R[k][k] >= 0) ? -norm : norm;
        double beta = norm * (norm + qAbs(m_R[k][k]));

        /* 构造Householder向量: v = R[k:m, k] */
        m_R[k][k] -= alpha;

        if (qAbs(beta) < 1e-300) continue;

        /* 应用Householder变换到R的右侧列: R = H * R */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = k; i < m; ++i) {
                dot += m_R[i][k] * m_R[i][j];
            }
            double coeff = dot / beta;
            for (int i = k; i < m; ++i) {
                m_R[i][j] -= coeff * m_R[i][k];
            }
        }

        /* 应用Householder变换到Q: Q = Q * H^T = Q * H */
        for (int j = 0; j < m; ++j) {
            double dot = 0.0;
            for (int i = k; i < m; ++i) {
                dot += m_R[i][k] * m_Q[j][i];
            }
            double coeff = dot / beta;
            for (int i = k; i < m; ++i) {
                m_Q[j][i] -= coeff * m_R[i][k];
            }
        }

        /* 恢复对角元素为精确值 */
        m_R[k][k] = alpha;
    }

    /* 清零R的下三角部分(消除数值误差) */
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < qMin(i, n); ++j) {
            m_R[i][j] = 0.0;
        }
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalDecompositions++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves);

    emit decompositionCompleted(m, n);
    return true;
}

/**
 * @brief 使用QR分解求解线性方程组Ax=b
 * @param b 右端项
 * @return 解向量x
 *
 * 利用QR分解结果求解Ax=b等价于R*x = Q^T*b。
 * 先计算Q^T*b，再对上三角矩阵R进行回代。
 */
QVector<double> SparseGMRES2::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_cols, 0.0);
    if (m_Q.isEmpty() || m_R.isEmpty()) return x;

    int m = m_rows;
    int n = m_cols;

    /* 步骤1: 计算 Q^T * b */
    QVector<double> Qtb(m, 0.0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            Qtb[i] += m_Q[i][j] * ((j < b.size()) ? b[j] : 0.0);
        }
    }

    /* 步骤2: 回代 R * x = Qtb */
    for (int i = qMin(n, m) - 1; i >= 0; --i) {
        double sum = Qtb[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= m_R[i][j] * x[j];
        }
        x[i] = (qAbs(m_R[i][i]) > 1e-15) ? sum / m_R[i][i] : 0.0;
    }

    /* 步骤3: 计算残差范数 */
    m_residual = 0.0;
    for (int i = n; i < m; ++i) {
        m_residual += Qtb[i] * Qtb[i];
    }
    m_residual = qSqrt(m_residual);

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves);

    return x;
}

/**
 * @brief 重置统计信息
 */
void SparseGMRES2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算矩阵的条件数估计
 * @return 条件数估计值（R对角线最大/最小比值）
 */
double SparseGMRES2::conditionNumber() const
{
    if (m_R.isEmpty()) return 1.0;

    int minDim = qMin(m_rows, m_cols);
    double maxDiag = 0.0, minDiag = 1e18;

    for (int i = 0; i < minDim; ++i) {
        double d = qAbs(m_R[i][i]);
        maxDiag = qMax(maxDiag, d);
        minDiag = qMin(minDiag, d);
    }

    return (minDiag > 1e-300) ? maxDiag / minDiag : 1e18;
}

/**
 * @brief 计算矩阵的有效秩
 * @param tol 容差阈值
 * @return 有效秩（R对角线中大于tol*max(|diag|)的元素个数）
 */
int SparseGMRES2::effectiveRank(double tol) const
{
    if (m_R.isEmpty()) return 0;

    int minDim = qMin(m_rows, m_cols);
    double maxDiag = 0.0;
    for (int i = 0; i < minDim; ++i) {
        maxDiag = qMax(maxDiag, qAbs(m_R[i][i]));
    }

    double threshold = tol * maxDiag;
    int rank = 0;
    for (int i = 0; i < minDim; ++i) {
        if (qAbs(m_R[i][i]) > threshold) rank++;
    }
    return rank;
}
