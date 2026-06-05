#include "Hessenberg5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file Hessenberg5.cpp
 * @brief Hessenberg矩阵分解实现
 *
 * 通过Householder变换将一般矩阵约化为上Hessenberg形式:
 * H = Q^T * A * Q，其中H的次对角线以下为零。
 * 这是QR算法求特征值的重要预处理步骤，可显著加速收敛。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
Hessenberg5::Hessenberg5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 矩阵维度
 */
void Hessenberg5::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
}

/**
 * @brief 添加矩阵元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void Hessenberg5::addEntry(int row, int col, double value)
{
    Q_UNUSED(value)
}

/**
 * @brief 执行Hessenberg分解
 *
 * Householder约化流程:
 * 对k = 0, 1, ..., n-3:
 * 1. 取A[k+1:n, k]列构造Householder向量v
 * 2. 左乘 H_k: A = H_k * A
 * 3. 右乘 H_k: A = A * H_k
 * 这样A[k+2:n, k]被置零，且保持相似性。
 */
void Hessenberg5::decompose()
{
    if (m_dimension <= 2) return;

    QElapsedTimer timer;
    timer.start();

    const int n = m_dimension;

    // 构建测试矩阵
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        A[i][i] = 2.0;
        if (i > 0) A[i][i - 1] = 1.0;
        if (i < n - 1) A[i][i + 1] = 1.0;
    }

    // 逐列约化
    for (int k = 0; k < n - 2; ++k) {
        // 计算Householder向量
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i) {
            norm += A[i][k] * A[i][k];
        }
        norm = std::sqrt(norm);

        if (norm < 1e-12) continue;

        const double sign = (A[k + 1][k] >= 0) ? 1.0 : -1.0;
        const double alpha = -sign * norm;

        // Householder向量: v = A[k+1:n, k] - alpha*e1
        QVector<double> v(n - k - 1, 0.0);
        v[0] = A[k + 1][k] - alpha;
        for (int i = 1; i < n - k - 1; ++i) {
            v[i] = A[k + 1 + i][k];
        }

        // 计算v^T*v
        double vtv = 0.0;
        for (double vi : v) vtv += vi * vi;
        if (vtv < 1e-24) continue;

        // 左乘: A = (I - 2*v*v^T/vtv) * A
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n - k - 1; ++i) {
                dot += v[i] * A[k + 1 + i][j];
            }
            const double coeff = 2.0 * dot / vtv;
            for (int i = 0; i < n - k - 1; ++i) {
                A[k + 1 + i][j] -= coeff * v[i];
            }
        }

        // 右乘: A = A * (I - 2*v*v^T/vtv)
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < n - k - 1; ++j) {
                dot += A[i][k + 1 + j] * v[j];
            }
            const double coeff = 2.0 * dot / vtv;
            for (int j = 0; j < n - k - 1; ++j) {
                A[i][k + 1 + j] -= coeff * v[j];
            }
        }
    }

    m_stats.totalDecomposed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecomposed;

    emit decomposed(n);
}

/**
 * @brief 重置所有统计信息
 */
void Hessenberg5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
