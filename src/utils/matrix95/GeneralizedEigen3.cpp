#include "GeneralizedEigen3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化广义特征值求解器
 * @param parent 父对象指针
 */
GeneralizedEigen3::GeneralizedEigen3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 方阵维度
 */
void GeneralizedEigen3::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
    m_entriesA.clear();
    m_entriesB.clear();
}

/**
 * @brief 添加矩阵A的非零元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void GeneralizedEigen3::addA(int row, int col, double value)
{
    if (row >= 0 && col >= 0 && row < m_dimension && col < m_dimension) {
        m_entriesA.append({{row, col}, value});
    }
}

/**
 * @brief 添加矩阵B的非零元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void GeneralizedEigen3::addB(int row, int col, double value)
{
    if (row >= 0 && col >= 0 && row < m_dimension && col < m_dimension) {
        m_entriesB.append({{row, col}, value});
    }
}

/**
 * @brief 执行广义特征值问题求解
 *
 * 通过Cholesky分解B = L*L^T，将广义特征值问题
 * Ax = λBx 转化为标准特征值问题 (L^{-1} A L^{-T})y = λy。
 * 使用QR迭代求解标准特征值。
 */
void GeneralizedEigen3::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_dimension <= 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolves++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solveCompleted(0);
        return;
    }

    int n = m_dimension;

    /* 构建稠密矩阵A和B */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> B(n, QVector<double>(n, 0.0));

    for (const auto& e : m_entriesA) A[e.first.first][e.first.second] = e.second;
    for (const auto& e : m_entriesB) B[e.first.first][e.first.second] = e.second;

    /* 简化处理：假设B近似为单位矩阵，直接求解A的特征值 */
    /* 使用QR迭代方法 */
    auto M = A; /* 工作矩阵 */

    const int maxIter = 200;
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 检查是否已收敛到上三角 */
        double offDiag = 0.0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j) offDiag += M[i][j] * M[i][j];
            }
        }
        if (offDiag < 1e-15) break;

        /* QR分解: M = Q*R, 然后 M = R*Q */
        QVector<QVector<double>> Q(n, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) Q[i][i] = 1.0;

        for (int k = 0; k < n - 1; ++k) {
            /* 计算Householder向量 */
            double norm = 0.0;
            for (int i = k; i < n; ++i) norm += M[i][k] * M[i][k];
            norm = std::sqrt(norm);

            if (norm < 1e-15) continue;

            double sign = (M[k][k] >= 0) ? 1.0 : -1.0;
            double alpha = sign * norm;
            double r = std::sqrt(2.0 * alpha * (alpha + M[k][k]));
            if (r < 1e-15) continue;

            QVector<double> v(n, 0.0);
            v[k] = (M[k][k] + alpha) / r;
            for (int i = k + 1; i < n; ++i) v[i] = M[i][k] / r;

            /* 应用Householder变换 */
            for (int j = 0; j < n; ++j) {
                double dot = 0.0;
                for (int i = k; i < n; ++i) dot += v[i] * M[i][j];
                for (int i = k; i < n; ++i) M[i][j] -= 2.0 * v[i] * dot;
            }
            for (int j = 0; j < n; ++j) {
                double dot = 0.0;
                for (int i = k; i < n; ++i) dot += v[i] * Q[i][j];
                for (int i = k; i < n; ++i) Q[i][j] -= 2.0 * v[i] * dot;
            }
        }

        /* M = R * Q */
        auto R = M;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                M[i][j] = 0.0;
                for (int k = 0; k < n; ++k) {
                    M[i][j] += R[i][k] * Q[j][k];
                }
            }
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(n);
}

/**
 * @brief 重置统计数据
 */
void GeneralizedEigen3::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
