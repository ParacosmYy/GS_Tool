#include "SparseLU3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化稀疏LU分解求解器
 * @param parent 父对象指针
 */
SparseLU3::SparseLU3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 矩阵维度(方阵)
 */
void SparseLU3::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
    m_entries.clear();
}

/**
 * @brief 添加稀疏矩阵非零元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void SparseLU3::addEntry(int row, int col, double value)
{
    if (row >= 0 && col >= 0 && row < m_dimension && col < m_dimension) {
        m_entries.append({{row, col}, value});
    }
}

/**
 * @brief 将稀疏矩阵转换为稠密矩阵
 * @param dim 维度
 * @param entries 稀疏元素列表
 * @return 稠密矩阵
 */
static QVector<QVector<double>> toDense(int dim,
    const QVector<QPair<QPair<int, int>, double>>& entries)
{
    QVector<QVector<double>> mat(dim, QVector<double>(dim, 0.0));
    for (const auto& e : entries) {
        mat[e.first.first][e.first.second] = e.second;
    }
    return mat;
}

/**
 * @brief 求解稀疏线性方程组 Ax=b
 *
 * 将稀疏矩阵转换为稠密表示，执行带部分主元的LU分解，
 * 然后通过前代和回代求解三角形方程组。
 *
 * @param rhs 右端向量b
 * @return 解向量x
 */
QVector<double> SparseLU3::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x;
    if (m_dimension <= 0 || rhs.size() != m_dimension) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolves++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solveCompleted(0, 0.0);
        return x;
    }

    int n = m_dimension;
    auto A = toDense(n, m_entries);

    /* LU分解(部分主元法) */
    QVector<int> piv(n);
    for (int i = 0; i < n; ++i) piv[i] = i;

    for (int k = 0; k < n; ++k) {
        /* 找主元 */
        int maxRow = k;
        double maxVal = std::abs(A[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (std::abs(A[i][k]) > maxVal) { maxVal = std::abs(A[i][k]); maxRow = i; }
        }
        if (maxRow != k) {
            std::swap(A[k], A[maxRow]);
            std::swap(piv[k], piv[maxRow]);
        }

        if (std::abs(A[k][k]) < 1e-15) continue;

        /* 消元 */
        for (int i = k + 1; i < n; ++i) {
            A[i][k] /= A[k][k];
            for (int j = k + 1; j < n; ++j) {
                A[i][j] -= A[i][k] * A[k][j];
            }
        }
    }

    /* 前代 Ly = Pb */
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        y[i] = rhs[piv[i]];
        for (int j = 0; j < i; ++j) y[i] -= A[i][j] * y[j];
    }

    /* 回代 Ux = y */
    x.resize(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j) x[i] -= A[i][j] * x[j];
        if (std::abs(A[i][i]) > 1e-15) x[i] /= A[i][i];
    }

    /* 计算残差 */
    double residual = 0.0;
    auto origA = toDense(n, m_entries);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) sum += origA[i][j] * x[j];
        residual += (sum - rhs[i]) * (sum - rhs[i]);
    }
    residual = std::sqrt(residual);

    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(n, residual);
    return x;
}

/**
 * @brief 重置统计数据
 */
void SparseLU3::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
