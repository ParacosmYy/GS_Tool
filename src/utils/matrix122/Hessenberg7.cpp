#include "Hessenberg7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Hessenberg约化求解器
 * @param parent 父对象指针
 */
Hessenberg7::Hessenberg7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Hessenberg7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 执行Hessenberg约化 A = Q H Q^T（Arnoldi过程）
 *
 * Arnoldi算法通过Krylov子空间投影生成正交基，
 * 将一般矩阵约化为上Hessenberg形式。
 * 过程：v1=b/||b||，逐次计算 Av_i 并正交化得到v_{i+1}和H的元素。
 *
 * @param matrix 输入矩阵
 * @return 是否约化成功
 */
bool Hessenberg7::reduce(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = matrix.size();
    if (n == 0) {
        emit reductionCompleted(0);
        return false;
    }

    /* 验证矩阵为方阵 */
    for (int i = 0; i < n; ++i) {
        if (matrix[i].size() != n) {
            emit reductionCompleted(0);
            return false;
        }
    }

    m_H.resize(n, QVector<double>(n, 0.0));
    m_Q.resize(n, QVector<double>(n, 0.0));

    /* Arnoldi过程 */
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));

    /* 初始向量 v1 = e1 */
    V[0][0] = 1.0;

    for (int j = 0; j < n; ++j) {
        /* 计算 w = A * v_j */
        QVector<double> w(n, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < n; ++k)
                w[i] += matrix[i][k] * V[j][k];
        }

        /* 正交化：减去之前所有基向量的投影 */
        for (int i = 0; i <= j; ++i) {
            double dot = 0.0;
            for (int k = 0; k < n; ++k)
                dot += w[k] * V[i][k];
            m_H[i][j] = dot;
            for (int k = 0; k < n; ++k)
                w[k] -= dot * V[i][k];
        }

        /* 计算下一个基向量的范数 */
        double normW = 0.0;
        for (int k = 0; k < n; ++k)
            normW += w[k] * w[k];
        normW = qSqrt(normW);

        if (j + 1 < n) {
            m_H[j + 1][j] = normW;
            if (normW > 1e-15) {
                for (int k = 0; k < n; ++k)
                    V[j + 1][k] = w[k] / normW;
            }
        }
    }

    /* Q的列即为Arnoldi向量 */
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            m_Q[i][j] = V[j][i];

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalReductions++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalReductions;

    emit reductionCompleted(n);
    return true;
}

/**
 * @brief 获取Hessenberg矩阵H
 * @return 上Hessenberg矩阵
 */
QVector<QVector<double>> Hessenberg7::hessenbergMatrix() const
{
    return m_H;
}

/**
 * @brief 获取正交变换矩阵Q
 * @return 正交矩阵
 */
QVector<QVector<double>> Hessenberg7::transformMatrix() const
{
    return m_Q;
}

/**
 * @brief 检查矩阵是否已经是Hessenberg形式
 *
 * 检查所有i > j+1位置的元素是否小于容差阈值。
 *
 * @param matrix 待检查矩阵
 * @param tolerance 数值容差
 * @return 是否为Hessenberg矩阵
 */
bool Hessenberg7::isHessenberg(const QVector<QVector<double>>& matrix,
                                double tolerance) const
{
    const int n = matrix.size();
    if (n == 0) return true;

    for (int i = 2; i < n; ++i) {
        for (int j = 0; j < i - 1; ++j) {
            if (qAbs(matrix[i][j]) > tolerance)
                return false;
        }
    }
    return true;
}
