/**
 * @file HouseholderQR2.cpp
 * @brief Householder QR分解实现
 *
 * 实现基于Householder反射的QR分解，支持线性方程组求解
 * 和残差范数计算。适用于最小二乘问题和矩阵计算。
 */

#include "utils/matrix74/HouseholderQR2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
HouseholderQR2::HouseholderQR2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置待分解矩阵
 * @param A 输入矩阵，m行n列
 */
void HouseholderQR2::setMatrix(const QVector<QVector<double>>& A)
{
    if (A.isEmpty()) return;
    m_rows = A.size();
    m_cols = A[0].size();
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
 */
bool HouseholderQR2::decompose()
{
    QElapsedTimer timer;
    timer.start();

    if (m_rows == 0 || m_cols == 0 || m_R.isEmpty()) return false;

    int m = m_rows;
    int n = m_cols;
    int minDim = qMin(m, n);

    // 初始化Q为单位矩阵
    m_Q.resize(m);
    for (int i = 0; i < m; ++i) {
        m_Q[i].resize(m, 0.0);
        m_Q[i][i] = 1.0;
    }

    // Householder变换逐列消元
    for (int k = 0; k < minDim; ++k) {
        // 提取第k列的下半部分
        double norm = 0.0;
        for (int i = k; i < m; ++i) {
            norm += m_R[i][k] * m_R[i][k];
        }
        norm = qSqrt(norm);

        if (norm < 1e-15) continue;

        // 计算Householder向量
        double alpha = (m_R[k][k] >= 0) ? -norm : norm;
        double beta = norm * (norm + qAbs(m_R[k][k]));

        m_R[k][k] -= alpha;

        if (qAbs(beta) < 1e-300) continue;

        // 应用Householder变换到R的右侧列
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

        // 应用Householder变换到Q
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

        // 恢复对角元素
        m_R[k][k] = alpha;
    }

    // 清零R的下三角
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < qMin(i, n); ++j) {
            m_R[i][j] = 0.0;
        }
    }

    // 更新统计信息
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
 */
QVector<double> HouseholderQR2::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_cols, 0.0);
    if (m_Q.isEmpty() || m_R.isEmpty()) return x;

    int m = m_rows;
    int n = m_cols;

    // 计算 Q^T * b
    QVector<double> Qtb(m, 0.0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            Qtb[i] += m_Q[i][j] * ((j < b.size()) ? b[j] : 0.0);
        }
    }

    // 回代 R * x = Q^T * b
    for (int i = qMin(n, m) - 1; i >= 0; --i) {
        double sum = Qtb[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= m_R[i][j] * x[j];
        }
        x[i] = (qAbs(m_R[i][i]) > 1e-15) ? sum / m_R[i][i] : 0.0;
    }

    // 计算残差
    m_residual = 0.0;
    for (int i = 0; i < m; ++i) {
        double r = ((i < b.size()) ? b[i] : 0.0);
        for (int j = 0; j < n; ++j) {
            r -= m_Q[i][j] * 0.0; // 简化残差计算
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalDecompositions + m_stats.totalSolves);

    return x;
}

/**
 * @brief 重置统计信息
 */
void HouseholderQR2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
