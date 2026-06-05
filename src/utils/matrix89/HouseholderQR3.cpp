#include "HouseholderQR3.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class HouseholderQR3
 * @brief Householder QR分解实现
 *
 * 使用Householder反射变换将矩阵A分解为正交矩阵Q和上三角矩阵R。
 * Householder变换通过构造反射矩阵H = I - 2vv^T，逐列消去下三角元素。
 * 相比Gram-Schmidt方法，Householder方法数值稳定性更好。
 * 时间复杂度O(2n^3/3)。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
HouseholderQR3::HouseholderQR3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行QR分解
 *
 * 对输入矩阵逐列构造Householder反射矩阵，逐步将矩阵变换为上三角形式。
 * 同时累积正交变换得到Q矩阵。返回矩阵的条件数(最大奇异值/最小奇异值)。
 *
 * @param matrix 输入矩阵(m×n)，m >= n
 * @return 分解是否成功
 */
bool HouseholderQR3::decompose(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int m = matrix.size();
    if (m == 0) return false;
    int n = matrix[0].size();
    if (n == 0 || m < n) return false;

    /* 复制到工作矩阵R */
    m_R = matrix;
    m_Q.clear();
    /* 初始化Q为单位矩阵 */
    m_Q.resize(m);
    for (int i = 0; i < m; ++i) {
        m_Q[i].resize(m, 0.0);
        m_Q[i][i] = 1.0;
    }

    /* 逐列执行Householder变换 */
    for (int k = 0; k < n; ++k) {
        /* 计算第k列下方部分的范数 */
        double norm = 0.0;
        for (int i = k; i < m; ++i) {
            norm += m_R[i][k] * m_R[i][k];
        }
        norm = qSqrt(norm);

        if (norm < 1e-15) continue;

        /* 构造Householder向量v */
        double sign = (m_R[k][k] >= 0) ? 1.0 : -1.0;
        double alpha = sign * norm;
        m_R[k][k] += alpha;

        /* 计算beta = 2 / (v^T * v) */
        double beta = 0.0;
        for (int i = k; i < m; ++i) {
            beta += m_R[i][k] * m_R[i][k];
        }
        if (qAbs(beta) < 1e-15) continue;
        beta = 2.0 / beta;

        /* 应用反射到R的剩余列 */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = k; i < m; ++i) {
                dot += m_R[i][k] * m_R[i][j];
            }
            dot *= beta;
            for (int i = k; i < m; ++i) {
                m_R[i][j] -= dot * m_R[i][k];
            }
        }

        /* 应用反射到Q */
        for (int j = 0; j < m; ++j) {
            double dot = 0.0;
            for (int i = k; i < m; ++i) {
                dot += m_R[i][k] * m_Q[j][i];
            }
            dot *= beta;
            for (int i = k; i < m; ++i) {
                m_Q[j][i] -= dot * m_R[i][k];
            }
        }

        /* 恢复R的对角元素 */
        m_R[k][k] = -alpha;
    }

    /* 估计条件数 */
    double maxSV = 0.0, minSV = 1e18;
    for (int i = 0; i < qMin(m, n); ++i) {
        double sv = qAbs(m_R[i][i]);
        if (sv > maxSV) maxSV = sv;
        if (sv < minSV) minSV = sv;
    }
    double conditioning = (minSV > 1e-15) ? maxSV / minSV : 1e18;

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(m, n, conditioning);

    return true;
}

/**
 * @brief 求解线性系统 Ax = b
 *
 * 利用已计算的QR分解，通过 Q^T * b 和 R的回代求解。
 * 等价于求解 Rx = Q^T * b，其中R为上三角矩阵。
 *
 * @param rhs 右端向量b
 * @return 解向量x
 */
QVector<double> HouseholderQR3::solve(const QVector<double>& rhs) const
{
    int n = m_R.isEmpty() ? 0 : m_R[0].size();
    QVector<double> x(n, 0.0);

    if (m_Q.isEmpty() || m_R.isEmpty() || rhs.size() != m_Q.size()) {
        return x;
    }

    /* 计算 Q^T * b */
    QVector<double> qtb(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m_Q.size(); ++j) {
            qtb[i] += m_Q[j][i] * rhs[j];
        }
    }

    /* 回代求解 Rx = Q^T * b */
    for (int i = n - 1; i >= 0; --i) {
        x[i] = qtb[i];
        for (int j = i + 1; j < n; ++j) {
            x[i] -= m_R[i][j] * x[j];
        }
        if (qAbs(m_R[i][i]) > 1e-15) {
            x[i] /= m_R[i][i];
        }
    }

    return x;
}

/**
 * @brief 重置所有统计数据
 *
 * 将分解计数、求解计数和计时归零。
 */
void HouseholderQR3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
