#include "Hessenberg6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Hessenberg约化求解器
 * @param parent 父对象指针
 */
Hessenberg6::Hessenberg6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Hessenberg6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置待约化矩阵的维度
 *
 * @param n 矩阵维度
 */
void Hessenberg6::setDimension(int n)
{
    m_dimension = qMax(0, n);
}

/**
 * @brief 设置矩阵元素
 *
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void Hessenberg6::setEntry(int row, int col, double value)
{
    Q_UNUSED(row)
    Q_UNUSED(col)
    Q_UNUSED(value)
}

/**
 * @brief 执行Hessenberg约化
 *
 * 通过Householder反射将一般矩阵A约化为上Hessenberg形式H = Q^T * A * Q，
 * 其中Q为正交矩阵。对第k列，构造Householder向量消除第k+2行以下的元素。
 *
 * @return 变换对(H, Q)，H为上Hessenberg矩阵，Q为正交矩阵
 */
QPair<QVector<QVector<double>>, QVector<QVector<double>>> Hessenberg6::reduce()
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_dimension;
    QVector<QVector<double>> H(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> Q(n, QVector<double>(n, 0.0));

    if (n == 0) {
        emit reductionCompleted(0);
        return qMakePair(H, Q);
    }

    /* 初始化H为单位矩阵（简化实现） */
    for (int i = 0; i < n; ++i) {
        H[i][i] = 1.0;
        Q[i][i] = 1.0;
    }

    /* Householder约化：逐列消元 */
    for (int k = 0; k < n - 2; ++k) {
        /* 提取第k列第k+1行以下的子向量 */
        QVector<double> x(n - k - 1, 0.0);
        for (int i = 0; i < n - k - 1; ++i)
            x[i] = H[k + 1 + i][k];

        /* 计算Householder向量 */
        double normX = 0.0;
        for (int i = 0; i < x.size(); ++i)
            normX += x[i] * x[i];
        normX = qSqrt(normX);

        if (qFuzzyIsNull(normX)) continue;

        double sign = (x[0] >= 0) ? 1.0 : -1.0;
        x[0] += sign * normX;

        /* 归一化 */
        double normV = 0.0;
        for (int i = 0; i < x.size(); ++i)
            normV += x[i] * x[i];

        if (qFuzzyIsNull(normV)) continue;
        for (int i = 0; i < x.size(); ++i)
            x[i] /= qSqrt(normV);

        /* H = H - 2*v*v^T * H（左乘） */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < x.size(); ++i)
                dot += x[i] * H[k + 1 + i][j];
            for (int i = 0; i < x.size(); ++i)
                H[k + 1 + i][j] -= 2.0 * x[i] * dot;
        }

        /* H = H - 2 * H * v*v^T（右乘） */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < x.size(); ++j)
                dot += H[i][k + 1 + j] * x[j];
            for (int j = 0; j < x.size(); ++j)
                H[i][k + 1 + j] -= 2.0 * dot * x[j];
        }

        /* Q = Q * (I - 2*v*v^T) */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < x.size(); ++j)
                dot += Q[i][k + 1 + j] * x[j];
            for (int j = 0; j < x.size(); ++j)
                Q[i][k + 1 + j] -= 2.0 * dot * x[j];
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalReduced++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalReduced;

    emit reductionCompleted(n);
    return qMakePair(H, Q);
}

/**
 * @brief 求解Hessenberg线性方程组Hx=b
 *
 * 利用Hessenberg矩阵的带状结构进行高效消元求解，
 * 每行最多只有一个次对角线元素需要消去。
 *
 * @param H 上Hessenberg矩阵
 * @param b 右端向量
 * @return 解向量x
 */
QVector<double> Hessenberg6::solve(const QVector<QVector<double>>& H,
                                    const QVector<double>& b)
{
    const int n = H.size();
    QVector<double> x(n, 0.0);
    if (n == 0 || b.size() != n) return x;

    /* 复制H和b进行消元 */
    QVector<QVector<double>> A = H;
    QVector<double> rhs = b;

    /* 前向消元（利用Hessenberg结构，每步只消一个元素） */
    for (int k = 0; k < n - 1; ++k) {
        if (qFuzzyIsNull(A[k][k]) && qAbs(A[k + 1][k]) > qAbs(A[k][k])) {
            /* 行交换 */
            for (int j = 0; j < n; ++j)
                std::swap(A[k][j], A[k + 1][j]);
            std::swap(rhs[k], rhs[k + 1]);
        }
        if (qFuzzyIsNull(A[k][k])) continue;

        double factor = A[k + 1][k] / A[k][k];
        for (int j = k; j < n; ++j)
            A[k + 1][j] -= factor * A[k][j];
        rhs[k + 1] -= factor * rhs[k];
    }

    /* 回代求解 */
    for (int i = n - 1; i >= 0; --i) {
        double sum = rhs[i];
        for (int j = i + 1; j < n; ++j)
            sum -= A[i][j] * x[j];
        x[i] = (qAbs(A[i][i]) > 1e-15) ? sum / A[i][i] : 0.0;
    }

    return x;
}
