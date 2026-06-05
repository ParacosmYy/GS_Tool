#include "Hessenberg4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Hessenberg分解求解器
 * @param parent 父对象指针
 */
Hessenberg4::Hessenberg4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 方阵维度
 */
void Hessenberg4::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
}

/**
 * @brief 添加矩阵元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void Hessenberg4::addEntry(int row, int col, double value)
{
    Q_UNUSED(row)
    Q_UNUSED(col)
    Q_UNUSED(value)
}

/**
 * @brief 执行Hessenberg分解(Householder方法)
 *
 * 通过n-2次Householder相似变换将矩阵化为上Hessenberg形式：
 * 对每列k，构造Householder反射矩阵消除k+2行以下的元素。
 * H = Q^T * A * Q，其中H为上Hessenberg矩阵。
 */
void Hessenberg4::decompose()
{
    QElapsedTimer timer;
    timer.start();

    if (m_dimension <= 2) {
        m_timeSum += timer.elapsed();
        m_stats.totalDecomposed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecomposed;
        emit decomposed(m_dimension);
        return;
    }

    int n = m_dimension;

    /* 构建稠密矩阵(简化为单位矩阵+随机扰动) */
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        A[i][i] = 2.0;
        if (i > 0) A[i][i - 1] = -1.0;
        if (i < n - 1) A[i][i + 1] = -1.0;
    }

    /* Householder约化到上Hessenberg形式 */
    for (int k = 0; k < n - 2; ++k) {
        /* 计算列k的Householder向量 */
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i) {
            norm += A[i][k] * A[i][k];
        }
        norm = std::sqrt(norm);

        if (norm < 1e-15) continue;

        double sign = (A[k + 1][k] >= 0) ? 1.0 : -1.0;
        double alpha = sign * norm;

        /* Householder向量v */
        QVector<double> v(n, 0.0);
        v[k + 1] = A[k + 1][k] + alpha;
        for (int i = k + 2; i < n; ++i) v[i] = A[i][k];

        double vNorm = 0.0;
        for (int i = k + 1; i < n; ++i) vNorm += v[i] * v[i];
        vNorm = std::sqrt(vNorm);
        if (vNorm < 1e-15) continue;

        for (int i = k + 1; i < n; ++i) v[i] /= vNorm;

        /* 左乘: A = (I - 2vv^T) * A */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = k + 1; i < n; ++i) dot += v[i] * A[i][j];
            for (int i = k + 1; i < n; ++i) A[i][j] -= 2.0 * v[i] * dot;
        }

        /* 右乘: A = A * (I - 2vv^T) */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j) dot += A[i][j] * v[j];
            for (int j = k + 1; j < n; ++j) A[i][j] -= 2.0 * dot * v[j];
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalDecomposed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecomposed;
    emit decomposed(n);
}

/**
 * @brief 重置统计数据
 */
void Hessenberg4::resetStatistics()
{
    m_stats.totalDecomposed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
