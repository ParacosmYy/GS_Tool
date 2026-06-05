/**
 * @file SchurDecomposition.cpp
 * @brief 实 Schur 分解实现(QR 迭代)
 */

#include "utils/schur/SchurDecomposition.h"

#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SchurDecomposition::SchurDecomposition(QObject* parent)
    : QObject(parent)
{
}

/** @brief 单位矩阵(n×n) */
static QVector<QVector<double>> identityMatrix(int n)
{
    QVector<QVector<double>> I(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        I[i][i] = 1.0;
    return I;
}

/** @brief 矩阵乘法 A×B */
static QVector<QVector<double>> matMul(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    const int n = A.size();
    const int m = B[0].size();
    const int p = B.size();
    QVector<QVector<double>> C(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i)
        for (int k = 0; k < p; ++k) {
            double aik = A[i][k];
            for (int j = 0; j < m; ++j)
                C[i][j] += aik * B[k][j];
        }
    return C;
}

/** @brief 矩阵转置 */
static QVector<QVector<double>> matTranspose(
    const QVector<QVector<double>>& A)
{
    const int rows = A.size();
    const int cols = A[0].size();
    QVector<QVector<double>> T(cols, QVector<double>(rows));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            T[j][i] = A[i][j];
    return T;
}

/**
 * @brief 从列向量构建 Householder 反射矩阵
 * @param v 列向量
 * @return H = I - 2vv^T/v^Tv
 */
static QVector<QVector<double>> householder(
    const QVector<double>& v)
{
    const int n = v.size();
    double dot = 0.0;
    for (int i = 0; i < n; ++i)
        dot += v[i] * v[i];
    if (std::abs(dot) < 1e-300)
        return identityMatrix(n);

    QVector<QVector<double>> H = identityMatrix(n);
    double scale = 2.0 / dot;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            H[i][j] -= scale * v[i] * v[j];
    return H;
}

/** @brief 提取矩阵第 k 列(从行 k 到末尾) */
static QVector<double> columnFrom(
    const QVector<QVector<double>>& A, int k)
{
    QVector<double> col;
    for (int i = k; i < static_cast<int>(A.size()); ++i)
        col.append(A[i][k]);
    return col;
}

/**
 * @brief 将矩阵约化为上 Hessenberg 形式(Householder 变换)
 * Schur 分解的预处理步骤，将一般矩阵变为上 Hessenberg 形。
 */
static QVector<QVector<double>> hessenbergReduce(
    QVector<QVector<double>> A)
{
    const int n = static_cast<int>(A.size());
    for (int k = 0; k < n - 2; ++k) {
        QVector<double> col = columnFrom(A, k + 1);
        double norm = 0.0;
        for (double val : col) norm += val * val;
        norm = std::sqrt(norm);
        if (norm < 1e-300) continue;

        double sign = (col[0] >= 0.0) ? 1.0 : -1.0;
        col[0] += sign * norm;
        double dot = 0.0;
        for (double val : col) dot += val * val;
        if (dot < 1e-300) continue;

        /* 对 A 左乘 H_k: A = (I - 2vv^T/dot) * A */
        for (int j = 0; j < n; ++j) {
            double proj = 0.0;
            for (int i = 0; i < static_cast<int>(col.size()); ++i)
                proj += col[i] * A[k + 1 + i][j];
            proj *= 2.0 / dot;
            for (int i = 0; i < static_cast<int>(col.size()); ++i)
                A[k + 1 + i][j] -= col[i] * proj;
        }

        /* 右乘 H_k: A = A * (I - 2vv^T/dot) */
        for (int i = 0; i < n; ++i) {
            double proj = 0.0;
            for (int j = 0; j < static_cast<int>(col.size()); ++j)
                proj += A[i][k + 1 + j] * col[j];
            proj *= 2.0 / dot;
            for (int j = 0; j < static_cast<int>(col.size()); ++j)
                A[i][k + 1 + j] -= proj * col[j];
        }
    }
    return A;
}

/**
 * @brief 执行实 Schur 分解
 *
 * 流程:
 *   1) Hessenberg 约化: A → H (上 Hessenberg)
 *   2) QR 迭代: 不断对 H 做 QR 分解 H=QR, 令 H=RQ,
 *      同时累积正交变换 Q_total。
 *   3) 迭代至 H 收敛为拟上三角(T)。
 */
QPair<QVector<QVector<double>>, QVector<QVector<double>>>
SchurDecomposition::decompose(QVector<QVector<double>> A,
                              int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    const int n = static_cast<int>(A.size());
    if (n == 0) {
        return qMakePair(QVector<QVector<double>>(),
                         QVector<QVector<double>>());
    }
    if (n == 1) {
        QVector<QVector<double>> Q = identityMatrix(1);
        return qMakePair(Q, A);
    }

    /* 步骤 1: Hessenberg 约化(此处简化，直接对原矩阵迭代) */
    QVector<QVector<double>> T = hessenbergReduce(A);
    QVector<QVector<double>> Qtotal = identityMatrix(n);

    /* 步骤 2: QR 迭代 */
    int iter = 0;
    for (iter = 0; iter < maxIter; ++iter) {
        /* Wilkinson 位移: 取右下 2×2 块的特征值中靠近 T[n-1][n-1] 的那个 */
        double shift = 0.0;
        if (n >= 2) {
            double a = T[n - 2][n - 2];
            double b = T[n - 2][n - 1];
            double c = T[n - 1][n - 2];
            double d = T[n - 1][n - 1];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = std::sqrt(std::max(0.0, tr * tr / 4.0 - det));
            double e1 = tr / 2.0 + disc;
            double e2 = tr / 2.0 - disc;
            shift = (std::abs(e1 - d) < std::abs(e2 - d)) ? e1 : e2;
        }

        /* T - shift·I */
        for (int i = 0; i < n; ++i)
            T[i][i] -= shift;

        /* 简化 QR: 使用 Givens 旋转 */
        QVector<QVector<double>> Qstep = identityMatrix(n);
        for (int i = 0; i < n - 1; ++i) {
            double a_val = T[i][i];
            double b_val = T[i + 1][i];
            double r = std::sqrt(a_val * a_val + b_val * b_val);
            if (r < 1e-300) continue;

            double cosT = a_val / r;
            double sinT = -b_val / r;

            /* 左乘 Givens: 更新 T 的第 i, i+1 行 */
            for (int j = 0; j < n; ++j) {
                double t1 = T[i][j];
                double t2 = T[i + 1][j];
                T[i][j]     = cosT * t1 - sinT * t2;
                T[i + 1][j] = sinT * t1 + cosT * t2;
            }
            /* 累积到 Qstep: 更新 Qstep 的第 i, i+1 列 */
            for (int j = 0; j < n; ++j) {
                double q1 = Qstep[j][i];
                double q2 = Qstep[j][i + 1];
                Qstep[j][i]     = cosT * q1 - sinT * q2;
                Qstep[j][i + 1] = sinT * q1 + cosT * q2;
            }
        }

        /* T = R·Q + shift·I (R 已经存在 T 中, 右乘 Qstep) */
        T = matMul(T, Qstep);
        for (int i = 0; i < n; ++i)
            T[i][i] += shift;

        /* 累积正交变换 */
        Qtotal = matMul(Qtotal, Qstep);

        /* 检查次对角线收敛 */
        double subDiagSum = 0.0;
        for (int i = 1; i < n; ++i)
            subDiagSum += std::abs(T[i][i - 1]);
        if (subDiagSum < kEpsilon * n)
            break;
    }

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;
    emit decompositionCompleted(n, iter);
    return qMakePair(Qtotal, T);
}

/** @brief 重置统计 */
void SchurDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
