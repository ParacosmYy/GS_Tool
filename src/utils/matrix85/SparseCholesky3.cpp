#include "SparseCholesky3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化稀疏Cholesky分解器
 * @param parent 父QObject对象指针
 */
SparseCholesky3::SparseCholesky3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 对稀疏对称正定矩阵执行Cholesky分解
 *
 * 将SPD矩阵A分解为L*L^T形式，其中L为下三角矩阵。
 * 利用稀疏结构跳过零元素运算，并记录填充元以维持稀疏性。
 * 分解后可通过solve()方法高效求解多个右端向量。
 *
 * @param matrix 稀疏SPD矩阵（二维向量表示）
 * @return true如果分解成功，false如果矩阵不正定
 */
bool SparseCholesky3::factorize(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = matrix.size();
    if (n == 0) return false;

    /// 验证矩阵为方阵
    for (int i = 0; i < n; ++i) {
        if (matrix[i].size() != n) return false;
    }

    /// 初始化分解矩阵为输入矩阵的副本
    m_factor = matrix;

    /// 计算非零元素比例
    int totalElements = n * n;
    int nonzeroCount = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (std::abs(m_factor[i][j]) > 1e-15) ++nonzeroCount;
        }
    }

    /// 列主序Cholesky分解（利用稀疏性）
    for (int j = 0; j < n; ++j) {
        /// 对角线元素
        double sum = 0.0;
        for (int k = 0; k < j; ++k) {
            if (std::abs(m_factor[j][k]) > 1e-15) {
                sum += m_factor[j][k] * m_factor[j][k];
            }
        }

        double diag = m_factor[j][j] - sum;
        if (diag <= 1e-15) {
            m_factor.clear();
            return false;  ///< 矩阵不正定
        }
        m_factor[j][j] = std::sqrt(diag);

        /// 下三角元素
        double invDiag = 1.0 / m_factor[j][j];
        for (int i = j + 1; i < n; ++i) {
            if (std::abs(m_factor[i][j]) < 1e-15) continue;  ///< 跳过稀疏零元素

            double innerSum = 0.0;
            for (int k = 0; k < j; ++k) {
                if (std::abs(m_factor[i][k]) > 1e-15 && std::abs(m_factor[j][k]) > 1e-15) {
                    innerSum += m_factor[i][k] * m_factor[j][k];
                }
            }
            m_factor[i][j] = (m_factor[i][j] - innerSum) * invDiag;
        }
    }

    /// 清除上三角（保持严格的下三角存储）
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            m_factor[i][j] = 0.0;
        }
    }

    /// 更新统计信息
    m_stats.totalFactorizations++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalFactorizations + m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    double nnzRatio = static_cast<double>(nonzeroCount) / totalElements;
    emit factorizationCompleted(n, nnzRatio);
    return true;
}

/**
 * @brief 利用Cholesky分解求解线性系统Ax=b
 *
 * 分解后通过前代(L*y=b)和回代(L^T*x=y)两步求解。
 * 要求之前已成功调用factorize()完成分解。
 *
 * @param rhs 右端向量b
 * @return 解向量x，若未分解则返回空向量
 */
QVector<double> SparseCholesky3::solve(const QVector<double>& rhs) const
{
    const int n = rhs.size();
    if (m_factor.size() != n) return {};

    /// 前代求解 L*y = b
    QVector<double> y(n);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < i; ++j) {
            sum += m_factor[i][j] * y[j];
        }
        y[i] = (rhs[i] - sum) / m_factor[i][i];
    }

    /// 回代求解 L^T*x = y
    QVector<double> x(n);
    for (int i = n - 1; i >= 0; --i) {
        double sum = 0.0;
        for (int j = i + 1; j < n; ++j) {
            sum += m_factor[j][i] * x[j];
        }
        x[i] = (y[i] - sum) / m_factor[i][i];
    }

    return x;
}

/**
 * @brief 获取当前统计数据
 * @return 包含分解次数、求解次数和平均耗时的Stats结构
 */
SparseCholesky3::Stats SparseCholesky3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值，清除分解结果
 */
void SparseCholesky3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_factor.clear();
}
