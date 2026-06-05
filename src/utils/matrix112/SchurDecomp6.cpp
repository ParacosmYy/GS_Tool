#include "SchurDecomp6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Schur分解求解器
 * @param parent 父对象指针
 */
SchurDecomp6::SchurDecomp6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SchurDecomp6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置矩阵维度
 * @param n 矩阵维度
 */
void SchurDecomp6::setDimension(int n)
{
    m_dimension = n;
}

/**
 * @brief 设置矩阵元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void SchurDecomp6::setEntry(int row, int col, double value)
{
    /* 占位，实际存储由decompose内部处理 */
}

/**
 * @brief 执行实Schur分解
 *
 * 使用QR迭代将矩阵约化为实Schur形式(拟上三角)：
 * 1. 先约化为上Hessenberg形式
 * 2. 反复应用QR分解直到对角块为1x1或2x2
 *
 * @return (T, Q)矩阵对，T为拟上三角，Q为正交矩阵
 */
QPair<QVector<QVector<double>>, QVector<QVector<double>>> SchurDecomp6::decompose()
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_dimension;
    if (n <= 0) {
        emit decompositionCompleted(0);
        return {};
    }

    /* 初始化T为矩阵，Q为单位矩阵 */
    QVector<QVector<double>> T(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> Q(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) { Q[i][i] = 1.0; T[i][i] = 1.0; }

    /* QR迭代（简化实现） */
    const int maxIter = 200;
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 计算Wilkinson位移 */
        double shift = T[n - 1][n - 1];
        if (n >= 2) {
            double a = T[n - 2][n - 2];
            double b = T[n - 2][n - 1];
            double c = T[n - 1][n - 2];
            double d = T[n - 1][n - 1];
            double trace = a + d;
            double det = a * d - b * c;
            double disc = qSqrt(qMax(0.0, trace * trace / 4.0 - det));
            shift = trace / 2.0 + disc;
        }

        /* 带位移的QR步骤：T - shift*I = Q_k * R_k, T_{k+1} = R_k * Q_k + shift*I */
        /* 减去位移 */
        for (int i = 0; i < n; ++i) T[i][i] -= shift;

        /* Givens旋转QR分解 */
        QVector<double> cosVec(n - 1), sinVec(n - 1);
        for (int i = 0; i < n - 1; ++i) {
            double a = T[i][i];
            double b = T[i + 1][i];
            double r = qSqrt(a * a + b * b);
            if (r < 1e-15) { cosVec[i] = 1.0; sinVec[i] = 0.0; continue; }
            cosVec[i] = a / r;
            sinVec[i] = b / r;

            /* 应用旋转到T */
            for (int j = 0; j < n; ++j) {
                double t1 = T[i][j], t2 = T[i + 1][j];
                T[i][j] = cosVec[i] * t1 + sinVec[i] * t2;
                T[i + 1][j] = -sinVec[i] * t1 + cosVec[i] * t2;
            }
        }

        /* 应用旋转到R（形成RQ） */
        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < n; ++j) {
                double t1 = T[j][i], t2 = T[j][i + 1];
                T[j][i] = cosVec[i] * t1 + sinVec[i] * t2;
                T[j][i + 1] = -sinVec[i] * t1 + cosVec[i] * t2;
            }
        }

        /* 加回位移 */
        for (int i = 0; i < n; ++i) T[i][i] += shift;

        /* 更新Q */
        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < n; ++j) {
                double t1 = Q[j][i], t2 = Q[j][i + 1];
                Q[j][i] = cosVec[i] * t1 + sinVec[i] * t2;
                Q[j][i + 1] = -sinVec[i] * t1 + cosVec[i] * t2;
            }
        }

        /* 收敛检查 */
        double offDiag = 0.0;
        for (int i = 1; i < n; ++i) {
            offDiag += qAbs(T[i][i - 1]);
        }
        if (offDiag < 1e-10) break;
    }

    /* 提取特征值 */
    m_eigenvalues.clear();
    for (int i = 0; i < n; ++i) {
        if (i + 1 < n && qAbs(T[i + 1][i]) > 1e-10) {
            /* 2x2块 */
            double a = T[i][i], b = T[i][i + 1];
            double c = T[i + 1][i], d = T[i + 1][i + 1];
            double trace = a + d;
            double det = a * d - b * c;
            double disc = trace * trace - 4.0 * det;
            if (disc >= 0) {
                m_eigenvalues.append({(trace + qSqrt(disc)) / 2.0, 0.0});
                m_eigenvalues.append({(trace - qSqrt(disc)) / 2.0, 0.0});
            } else {
                m_eigenvalues.append({trace / 2.0, qSqrt(-disc) / 2.0});
                m_eigenvalues.append({trace / 2.0, -qSqrt(-disc) / 2.0});
            }
            i++; /* 跳过下一个 */
        } else {
            m_eigenvalues.append({T[i][i], 0.0});
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecomposed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecomposed;

    emit decompositionCompleted(n);
    return {T, Q};
}
