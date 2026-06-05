/**
 * @file ConditionNumber2.cpp
 * @brief 矩阵条件数计算器实现
 *
 * 计算矩阵在多种范数下的条件数:
 * - 1-范数 (列和最大值)
 * - 无穷范数 (行和最大值)
 * - Frobenius范数 (元素平方和的平方根)
 * 条件数 = ||A|| * ||A^{-1}||，衡量矩阵接近奇异的程度。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/matrix63/ConditionNumber2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化条件数计算器
 * @param parent 父QObject指针
 */
ConditionNumber2::ConditionNumber2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置待分析的矩阵
 * @param A 输入方阵 (必须为 n x n)
 */
void ConditionNumber2::setMatrix(const QVector<QVector<double>>& A)
{
    m_A = A;
    m_n = A.size();
}

/**
 * @brief 计算矩阵条件数
 *
 * 根据normType参数选择范数类型:
 * - normType=1: 使用1-范数 (列和最大值)
 * - normType=2: 使用Frobenius范数近似
 * - normType=3 或其他: 使用无穷范数 (行和最大值)
 *
 * 步骤:
 * 1. 计算矩阵A的指定范数
 * 2. 计算A的逆矩阵 A^{-1}
 * 3. 计算A^{-1}的指定范数
 * 4. 条件数 = ||A|| * ||A^{-1}||
 *
 * @param normType 范数类型 (1=1-范数, 2=F-范数, 其他=无穷范数)
 * @return 条件数值，值越大矩阵越接近奇异
 */
double ConditionNumber2::compute(int normType)
{
    QElapsedTimer timer;
    timer.start();

    m_cond = 0.0;

    if (m_n == 0 || m_A.isEmpty()) {
        emit computed(0, 0.0);
        return 0.0;
    }

    /* 检查矩阵是否为方阵 */
    for (int i = 0; i < m_n; ++i) {
        if (m_A[i].size() != m_n) {
            emit computed(m_n, 0.0);
            return 0.0;
        }
    }

    /* 计算矩阵A的范数 */
    m_norm1 = computeNorm1(m_A);
    m_normInf = computeNormInf(m_A);
    m_normFrob = computeNormFrob(m_A);

    /* 计算逆矩阵 (高斯-约当消元法) */
    QVector<QVector<double>> inv(m_n, QVector<double>(m_n, 0.0));
    for (int i = 0; i < m_n; ++i) inv[i][i] = 1.0;

    QVector<QVector<double>> aug = m_A;

    /* 前向消元 + 部分主元选取 */
    for (int col = 0; col < m_n; ++col) {
        /* 寻找列中绝对值最大的元素作为主元 */
        int maxRow = col;
        double maxVal = qAbs(aug[col][col]);
        for (int row = col + 1; row < m_n; ++row) {
            if (qAbs(aug[row][col]) > maxVal) {
                maxVal = qAbs(aug[row][col]);
                maxRow = row;
            }
        }

        /* 交换行 */
        if (maxRow != col) {
            std::swap(aug[col], aug[maxRow]);
            std::swap(inv[col], inv[maxRow]);
        }

        /* 检查是否奇异 */
        if (qAbs(aug[col][col]) < 1e-12) {
            m_cond = std::numeric_limits<double>::infinity();
            emit computed(m_n, m_cond);
            return m_cond;
        }

        /* 消元 */
        double pivot = aug[col][col];
        for (int j = 0; j < m_n; ++j) {
            aug[col][j] /= pivot;
            inv[col][j] /= pivot;
        }

        for (int row = 0; row < m_n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < m_n; ++j) {
                aug[row][j] -= factor * aug[col][j];
                inv[row][j] -= factor * inv[col][j];
            }
        }
    }

    /* 计算逆矩阵的范数 */
    double invNorm = 0.0;
    switch (normType) {
        case 1:
            invNorm = computeNorm1(inv);
            m_cond = m_norm1 * invNorm;
            break;
        case 2:
            invNorm = computeNormFrob(inv);
            m_cond = m_normFrob * invNorm;
            break;
        default:
            invNorm = computeNormInf(inv);
            m_cond = m_normInf * invNorm;
            break;
    }

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalDimensions += m_n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_n, m_cond);
    return m_cond;
}

/**
 * @brief 重置所有统计数据
 */
void ConditionNumber2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算1-范数 (列和最大值)
 *
 * 1-范数 = max_j sum_i |A[i][j]|
 *
 * @param A 输入矩阵
 * @return 1-范数值
 */
double ConditionNumber2::computeNorm1(const QVector<QVector<double>>& A) const
{
    const int n = A.size();
    if (n == 0) return 0.0;

    double maxColSum = 0.0;
    for (int j = 0; j < n; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < n; ++i) {
            colSum += qAbs(A[i][j]);
        }
        maxColSum = qMax(maxColSum, colSum);
    }
    return maxColSum;
}

/**
 * @brief 计算无穷范数 (行和最大值)
 *
 * 无穷范数 = max_i sum_j |A[i][j]|
 *
 * @param A 输入矩阵
 * @return 无穷范数值
 */
double ConditionNumber2::computeNormInf(const QVector<QVector<double>>& A) const
{
    const int n = A.size();
    if (n == 0) return 0.0;

    double maxRowSum = 0.0;
    for (int i = 0; i < n; ++i) {
        double rowSum = 0.0;
        for (int j = 0; j < static_cast<int>(A[i].size()); ++j) {
            rowSum += qAbs(A[i][j]);
        }
        maxRowSum = qMax(maxRowSum, rowSum);
    }
    return maxRowSum;
}

/**
 * @brief 计算Frobenius范数
 *
 * F-范数 = sqrt(sum_i sum_j A[i][j]^2)
 *
 * @param A 输入矩阵
 * @return Frobenius范数值
 */
double ConditionNumber2::computeNormFrob(const QVector<QVector<double>>& A) const
{
    const int n = A.size();
    if (n == 0) return 0.0;

    double sumSq = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < static_cast<int>(A[i].size()); ++j) {
            sumSq += A[i][j] * A[i][j];
        }
    }
    return qSqrt(sumSq);
}
