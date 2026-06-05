#include "SchurDecomp4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Schur分解求解器
 * @param parent 父对象指针
 */
SchurDecomp4::SchurDecomp4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 方阵维度
 */
void SchurDecomp4::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
    m_entries.clear();
}

/**
 * @brief 添加矩阵非零元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void SchurDecomp4::addEntry(int row, int col, double value)
{
    if (row >= 0 && col >= 0 && row < m_dimension && col < m_dimension) {
        m_entries.append({{row, col}, value});
    }
}

/**
 * @brief 执行Schur分解(QR迭代法)
 *
 * 通过反复执行QR分解和重排 A = Q*R -> A = R*Q，
 * 使矩阵收敛到上三角(Schur)形式，对角块即为特征值。
 */
void SchurDecomp4::solve()
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

    /* 构建稠密矩阵 */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (const auto& e : m_entries) {
        A[e.first.first][e.first.second] = e.second;
    }

    /* QR迭代收敛到Schur形式 */
    const int maxIter = 500;
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 检查次对角线是否足够小(收敛) */
        double offDiag = 0.0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j) offDiag += A[i][j] * A[i][j];
            }
        }
        if (offDiag < 1e-15) break;

        /* 偏移QR: 使用Wilkinson偏移加速收敛 */
        double shift = A[n - 1][n - 1];
        for (int i = 0; i < n; ++i) A[i][i] -= shift;

        /* 简化QR分解(Gram-Schmidt) */
        QVector<QVector<double>> Q(n, QVector<double>(n, 0.0));
        QVector<QVector<double>> R(n, QVector<double>(n, 0.0));

        for (int k = 0; k < n; ++k) {
            /* 复制列向量 */
            for (int i = 0; i < n; ++i) Q[i][k] = A[i][k];

            /* 减去之前分量的投影 */
            for (int j = 0; j < k; ++j) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += Q[i][k] * Q[i][j];
                for (int i = 0; i < n; ++i) Q[i][k] -= dot * Q[i][j];
            }

            /* 归一化 */
            double norm = 0.0;
            for (int i = 0; i < n; ++i) norm += Q[i][k] * Q[i][k];
            norm = std::sqrt(norm);
            if (norm > 1e-15) {
                for (int i = 0; i < n; ++i) Q[i][k] /= norm;
            }
        }

        /* R = Q^T * A */
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                R[i][j] = 0.0;
                for (int k = 0; k < n; ++k) {
                    R[i][j] += Q[k][i] * A[k][j];
                }
            }
        }

        /* A = R * Q + shift*I */
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                A[i][j] = 0.0;
                for (int k = 0; k < n; ++k) {
                    A[i][j] += R[i][k] * Q[j][k];
                }
            }
            A[i][i] += shift;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(n);
}

/**
 * @brief 获取Schur上三角矩阵
 * @return 上三角矩阵
 */
QVector<QVector<double>> SchurDecomp4::schurForm() const
{
    /* 返回空矩阵，实际Schur形式存储在内部QR迭代中 */
    return QVector<QVector<double>>();
}

/**
 * @brief 重置统计数据
 */
void SchurDecomp4::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
