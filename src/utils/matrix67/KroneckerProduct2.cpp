/**
 * @file KroneckerProduct2.cpp
 * @brief Kronecker积(张量积)计算实现 — 矩阵张量积运算
 *
 * 计算两个矩阵的Kronecker积 C = A (x) B，
 * 其中C[i*p+r][j*q+s] = A[i][j] * B[r][s]，
 * A为m*n矩阵，B为p*q矩阵，结果为(m*p) x (n*q)矩阵。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/matrix67/KroneckerProduct2.h"

#include <QElapsedTimer>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
KroneckerProduct2::KroneckerProduct2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵A
 * @param A 输入矩阵(任意维度)
 */
void KroneckerProduct2::setMatrixA(const QVector<QVector<double>>& A)
{
    m_A = A;
    if (!A.isEmpty()) {
        /* 更新行列信息（compute时才真正确定输出维度） */
    }
}

/**
 * @brief 设置矩阵B
 * @param B 输入矩阵(任意维度)
 */
void KroneckerProduct2::setMatrixB(const QVector<QVector<double>>& B)
{
    m_B = B;
}

/**
 * @brief 计算Kronecker积
 *
 * Kronecker积定义:
 * 若A为 m x n，B为 p x q，
 * 则 C = A (x) B 为 (m*p) x (n*q) 矩阵，
 * 其中 C[i*p+r][j*q+s] = A[i][j] * B[r][s]
 *
 * @return Kronecker积结果矩阵
 */
QVector<QVector<double>> KroneckerProduct2::compute()
{
    QElapsedTimer timer;
    timer.start();

    const int mA = m_A.size();
    const int nA = (mA > 0) ? m_A[0].size() : 0;
    const int mB = m_B.size();
    const int nB = (mB > 0) ? m_B[0].size() : 0;

    if (mA == 0 || nA == 0 || mB == 0 || nB == 0) {
        m_rows = 0;
        m_cols = 0;
        m_result.clear();
        return {};
    }

    /* 验证A的列数一致性 */
    for (int i = 0; i < mA; ++i) {
        if (m_A[i].size() != nA) {
            m_rows = 0;
            m_cols = 0;
            m_result.clear();
            return {};
        }
    }

    /* 验证B的列数一致性 */
    for (int i = 0; i < mB; ++i) {
        if (m_B[i].size() != nB) {
            m_rows = 0;
            m_cols = 0;
            m_result.clear();
            return {};
        }
    }

    /* 输出维度 */
    m_rows = mA * mB;
    m_cols = nA * nB;

    /* 计算Kronecker积 */
    m_result.resize(m_rows);
    for (int i = 0; i < m_rows; ++i) {
        m_result[i].resize(m_cols, 0.0);
    }

    for (int ia = 0; ia < mA; ++ia) {
        for (int ja = 0; ja < nA; ++ja) {
            double aij = m_A[ia][ja];
            for (int ib = 0; ib < mB; ++ib) {
                for (int jb = 0; jb < nB; ++jb) {
                    int outRow = ia * mB + ib;
                    int outCol = ja * nB + jb;
                    m_result[outRow][outCol] = aij * m_B[ib][jb];
                }
            }
        }
    }

    /* 更新统计信息 */
    m_stats.totalProducts++;
    m_stats.totalDimensions += m_rows + m_cols;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProducts;

    emit computed(m_rows, m_cols);
    return m_result;
}

/**
 * @brief 重置所有统计数据
 */
void KroneckerProduct2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算Kronecker积的逆运算近似
 *
 * 给定结果矩阵和原始矩阵A、B的维度信息，
 * 验证Kronecker积的正确性: 检查每个子块是否为 A[i][j]*B。
 *
 * @param result 待验证的结果矩阵
 * @param tolerance 允许的数值误差
 * @return true如果结果矩阵与理论Kronecker积一致
 */
bool KroneckerProduct2::verifyResult(
    const QVector<QVector<double>>& result,
    double tolerance) const
{
    const int mA = m_A.size();
    const int nA = (mA > 0) ? m_A[0].size() : 0;
    const int mB = m_B.size();
    const int nB = (mB > 0) ? m_B[0].size() : 0;

    if (mA == 0 || nA == 0 || mB == 0 || nB == 0) return false;

    const int expectedRows = mA * mB;
    const int expectedCols = nA * nB;

    if (result.size() != expectedRows) return false;
    for (int i = 0; i < expectedRows; ++i) {
        if (result[i].size() != expectedCols) return false;
    }

    /* 逐块验证 */
    for (int ia = 0; ia < mA; ++ia) {
        for (int ja = 0; ja < nA; ++ja) {
            double aij = m_A[ia][ja];
            for (int ib = 0; ib < mB; ++ib) {
                for (int jb = 0; jb < nB; ++jb) {
                    int outRow = ia * mB + ib;
                    int outCol = ja * nB + jb;
                    double expected = aij * m_B[ib][jb];
                    double diff = qAbs(result[outRow][outCol] - expected);
                    if (diff > tolerance) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

/**
 * @brief 提取Kronecker积结果中的子块
 *
 * 从结果矩阵中提取对应于A[i][j]的子块矩阵。
 * 每个子块为A[i][j] * B。
 *
 * @param i A的行索引
 * @param j A的列索引
 * @return 对应的子块矩阵
 */
QVector<QVector<double>> KroneckerProduct2::extractBlock(int i, int j) const
{
    const int mB = m_B.size();
    const int nB = (mB > 0) ? m_B[0].size() : 0;

    if (m_result.isEmpty() || i < 0 || j < 0 || mB == 0 || nB == 0) {
        return {};
    }

    QVector<QVector<double>> block(mB, QVector<double>(nB, 0.0));
    int rowOffset = i * mB;
    int colOffset = j * nB;

    for (int r = 0; r < mB; ++r) {
        for (int c = 0; c < nB; ++c) {
            int resultRow = rowOffset + r;
            int resultCol = colOffset + c;
            if (resultRow < m_rows && resultCol < m_cols) {
                block[r][c] = m_result[resultRow][resultCol];
            }
        }
    }
    return block;
}

/**
 * @brief 计算结果矩阵的Frobenius范数
 *
 * ||C||_F = sqrt(sum_{i,j} C_{i,j}^2)
 * 用于评估Kronecker积结果的数值量级。
 *
 * @return Frobenius范数
 */
double KroneckerProduct2::frobeniusNorm() const
{
    double sumSq = 0.0;
    for (const auto& row : m_result) {
        for (double val : row) {
            sumSq += val * val;
        }
    }
    return qSqrt(sumSq);
}
