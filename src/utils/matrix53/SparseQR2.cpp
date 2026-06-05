/**
 * @file SparseQR2.cpp
 * @brief 稀疏矩阵QR分解实现
 *
 * 实现基于Givens旋转的稀疏矩阵QR分解。对于稀疏矩阵，
 * Givens旋转比Householder反射更适合，因为每次旋转只影响
 * 两行，能更好地保持稀疏结构。使用QElapsedTimer计时。
 */

#include "utils/matrix53/SparseQR2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

/**
 * @class SparseQR2
 * @brief 稀疏矩阵QR分解器，基于Givens旋转
 *
 * 将稀疏矩阵A分解为A = Q*R，其中Q为正交矩阵，R为上三角矩阵。
 * 分解后可高效求解线性方程组 Ax = b。支持矩阵秩计算和残差范数估计。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject指针
 */
SparseQR2::SparseQR2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置稀疏矩阵（CSR格式）
 * @param rows 行数
 * @param cols 列数
 * @param rowPtr 行指针数组，长度为rows+1
 * @param colIdx 列索引数组
 * @param values 非零值数组
 */
void SparseQR2::setMatrix(int rows, int cols, const QVector<int>& rowPtr,
                           const QVector<int>& colIdx, const QVector<double>& values)
{
    m_rows = rows;
    m_cols = cols;
    m_rowPtr = rowPtr;
    m_colIdx = colIdx;
    m_values = values;
    m_decomposed = false;
    m_rank = 0;
    m_resNorm = 0.0;
}

/**
 * @brief 执行QR分解
 *
 * 将稀疏矩阵转换为稠密存储后，使用Givens旋转逐步
 * 消去下三角元素。每次旋转产生一个2x2正交变换，作用于
 * 两行数据。旋转参数c和s保存在m_Qdiag中供后续求解使用。
 *
 * @return 分解成功返回true
 */
bool SparseQR2::decompose()
{
    QElapsedTimer timer;
    timer.start();

    if (m_rows <= 0 || m_cols <= 0) return false;

    const int minDim = qMin(m_rows, m_cols);

    /* 将CSR格式转为稠密R矩阵 */
    m_R.fill(0.0, m_rows * m_cols);
    for (int i = 0; i < m_rows; ++i) {
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1] && j < m_colIdx.size(); ++j) {
            int col = m_colIdx[j];
            if (col < m_cols) {
                m_R[i * m_cols + col] = m_values[j];
            }
        }
    }

    /* 保存旋转参数(Q的紧凑表示)：每对(c, s)对应一次Givens旋转 */
    m_Qdiag.fill(0.0, m_rows * m_cols * 2);

    /* 使用Givens旋转消去下三角元素 */
    for (int j = 0; j < m_cols && j < m_rows; ++j) {
        for (int i = j + 1; i < m_rows; ++i) {
            double a = m_R[j * m_cols + j];  /* 对角线元素 */
            double b = m_R[i * m_cols + j];  /* 待消去元素 */

            if (qAbs(b) < 1e-15) continue;

            /* 计算Givens旋转参数 */
            double r = qSqrt(a * a + b * b);
            double c = a / r;
            double s = -b / r;

            /* 保存旋转参数 */
            m_Qdiag[i * m_cols * 2 + j * 2] = c;
            m_Qdiag[i * m_cols * 2 + j * 2 + 1] = s;

            /* 对第j行和第i行执行旋转 */
            for (int k = j; k < m_cols; ++k) {
                double rj = m_R[j * m_cols + k];
                double ri = m_R[i * m_cols + k];
                m_R[j * m_cols + k] = c * rj - s * ri;
                m_R[i * m_cols + k] = s * rj + c * ri;
            }
        }
    }

    /* 计算矩阵的数值秩（对角线非零元素个数） */
    m_rank = 0;
    for (int i = 0; i < minDim; ++i) {
        if (qAbs(m_R[i * m_cols + i]) > 1e-10) {
            m_rank++;
        }
    }

    m_decomposed = true;

    m_stats.totalDecompositions++;
    m_stats.matrixSize = qMax(m_rows, m_cols);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions + m_stats.totalSolves > 0)
        ? m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves) : 0.0;

    emit decompositionCompleted(m_rows, m_cols, m_rank);
    return true;
}

/**
 * @brief 求解线性方程组 Ax = b
 *
 * 先将右端向量b通过Q^T变换(Q^T*b)，然后通过回代
 * 求解R*x = Q^T*b。若系统欠定（秩<列数），使用最小二乘解。
 *
 * @param rhs 右端向量，长度为m_rows
 * @return 解向量，长度为m_cols
 */
QVector<double> SparseQR2::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_cols, 0.0);

    if (!m_decomposed || rhs.size() < m_rows) {
        m_stats.totalSolves++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions + m_stats.totalSolves > 0)
            ? m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves) : 0.0;
        return x;
    }

    /* 计算 Q^T * b：应用所有Givens旋转到b */
    QVector<double> Qtb(m_rows, 0.0);
    for (int i = 0; i < qMin(rhs.size(), m_rows); ++i) {
        Qtb[i] = rhs[i];
    }

    for (int j = 0; j < m_cols && j < m_rows; ++j) {
        for (int i = j + 1; i < m_rows; ++i) {
            double c = m_Qdiag[i * m_cols * 2 + j * 2];
            double s = m_Qdiag[i * m_cols * 2 + j * 2 + 1];
            if (qAbs(c) < 1e-15 && qAbs(s) < 1e-15) continue;

            double bj = Qtb[j];
            double bi = Qtb[i];
            Qtb[j] = c * bj - s * bi;
            Qtb[i] = s * bj + c * bi;
        }
    }

    /* 回代求解 R * x = Q^T * b */
    for (int i = qMin(m_rank, m_cols) - 1; i >= 0; --i) {
        double sum = Qtb[i];
        for (int j = i + 1; j < m_cols; ++j) {
            sum -= m_R[i * m_cols + j] * x[j];
        }
        double diag = m_R[i * m_cols + i];
        x[i] = (qAbs(diag) > 1e-15) ? sum / diag : 0.0;
    }

    /* 计算残差范数 */
    m_resNorm = 0.0;
    for (int i = m_rank; i < m_rows; ++i) {
        m_resNorm += Qtb[i] * Qtb[i];
    }
    m_resNorm = qSqrt(m_resNorm);

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions + m_stats.totalSolves > 0)
        ? m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves) : 0.0;

    return x;
}

/**
 * @brief 重置统计数据
 */
void SparseQR2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
