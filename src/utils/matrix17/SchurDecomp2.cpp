/**
 * @file SchurDecomp2.cpp
 * @brief Schur分解(增强版)实现 — Hessenberg预处理 + Francis双移QR
 */

#include "utils/matrix17/SchurDecomp2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/* ========== 构造/配置 ========== */

SchurDecomp2::SchurDecomp2(QObject* parent)
    : QObject(parent), m_maxIter(500), m_epsilon(1e-12), m_timeSum(0.0) {}

void SchurDecomp2::setMaxIterations(int maxIter) { m_maxIter = qMax(10, maxIter); }
void SchurDecomp2::setEpsilon(double eps)         { m_epsilon = qMax(1e-15, eps); }

/* ========== 主分解 ========== */

SchurDecomp2::SchurResult SchurDecomp2::decompose(
    const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    SchurResult result;
    int n = A.size();
    if (n == 0) {
        result.converged = true;
        m_timeSum += timer.elapsed();
        return result;
    }

    /* 检查方阵 */
    for (int i = 0; i < n; ++i)
        if (A[i].size() != n) {
            result.converged = false;
            m_timeSum += timer.elapsed();
            return result;
        }

    /* 拷贝工作矩阵 */
    QVector<QVector<double>> H = A;
    QVector<QVector<double>> Q(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) Q[i][i] = 1.0;

    /* Step 1: Hessenberg归约 */
    hessenbergReduce(H, Q);

    /* Step 2: Francis双移QR迭代 */
    int p = n; /* 当前活跃子矩阵大小 */
    int totalIter = 0;
    bool converged = true;

    while (p > 2) {
        /* 检查底部元素是否足够小(可分裂) */
        if (qAbs(H[p - 1][p - 2]) <= m_epsilon *
            (qAbs(H[p - 2][p - 2]) + qAbs(H[p - 1][p - 1]))) {
            H[p - 1][p - 2] = 0.0;
            --p;
            continue;
        }

        /* 检查倒数第二个次对角元素 */
        if (p > 2 && qAbs(H[p - 2][p - 3]) <= m_epsilon *
            (qAbs(H[p - 3][p - 3]) + qAbs(H[p - 2][p - 2]))) {
            H[p - 2][p - 3] = 0.0;
            p -= 2;
            continue;
        }

        /* 执行Francis双移QR步 */
        int q_start = 0;
        /* 寻找最低不可忽略的次对角元素 */
        for (int i = p - 2; i >= 1; --i) {
            if (qAbs(H[i][i - 1]) <= m_epsilon *
                (qAbs(H[i - 1][i - 1]) + qAbs(H[i][i]))) {
                H[i][i - 1] = 0.0;
                q_start = i;
                break;
            }
        }

        francisDoubleShift(H, Q, q_start, p);

        ++totalIter;
        emit iterationStep(totalIter, qAbs(H[p - 1][p - 2]));

        if (totalIter >= m_maxIter) {
            converged = false;
            break;
        }
    }

    /* 处理残余的2x2块 */
    if (p == 2 && qAbs(H[1][0]) > m_epsilon) {
        /* 2x2块已经在上Hessenberg形中, 不需要额外处理 */
    }

    /* Step 3: 从拟上三角T提取特征值 */
    result.eigenvalues = extractEigenvalues(H);
    result.Q = Q;
    result.T = H;
    result.iterations = totalIter;
    result.converged = converged;

    /* 统计更新 */
    ++m_stats.totalDecompositions;
    m_stats.totalQRIterations += totalIter;
    m_stats.totalEigenvaluesExtracted += result.eigenvalues.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n, totalIter, converged);
    return result;
}

/* ========== 仅特征值 ========== */

QVector<SchurDecomp2::ComplexEigen> SchurDecomp2::eigenvaluesOnly(
    const QVector<QVector<double>>& A)
{
    SchurResult res = decompose(A);
    return res.eigenvalues;
}

/* ========== Hessenberg归约 ========== */

void SchurDecomp2::hessenbergReduce(
    QVector<QVector<double>>& A, QVector<QVector<double>>& Q) const
{
    int n = A.size();
    for (int k = 0; k < n - 2; ++k) {
        /* 计算Householder向量将A[k+1:n, k]归约 */
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i)
            norm += A[i][k] * A[i][k];
        norm = qSqrt(norm);

        if (norm < 1e-15) continue;

        double sign = (A[k + 1][k] >= 0) ? 1.0 : -1.0;
        double alpha = sign * norm;
        double denom = A[k + 1][k] + alpha;
        if (qAbs(denom) < 1e-15) continue;

        /* v = A[k+1:n, k] + alpha*e1, 归一化 */
        QVector<double> v(n - k - 1, 0.0);
        v[0] = 1.0; /* 隐式归一化 */
        double beta = -denom; /* 2/v^T v 的分母 */

        for (int i = 1; i < v.size(); ++i)
            v[i] = A[k + 1 + i][k] / denom;

        /* 左乘: A = (I - beta*v*v^T) * A */
        for (int j = k; j < n; ++j) {
            double dot = A[k + 1][j];
            for (int i = 1; i < v.size(); ++i)
                dot += v[i] * A[k + 1 + i][j];
            A[k + 1][j] -= beta * dot;
            for (int i = 1; i < v.size(); ++i)
                A[k + 1 + i][j] -= beta * v[i] * dot;
        }

        /* 右乘: A = A * (I - beta*v*v^T) */
        for (int i = 0; i < n; ++i) {
            double dot = A[i][k + 1];
            for (int j = 1; j < v.size(); ++j)
                dot += v[j] * A[i][k + 1 + j];
            A[i][k + 1] -= beta * dot;
            for (int j = 1; j < v.size(); ++j)
                A[i][k + 1 + j] -= beta * v[j] * dot;
        }

        /* 累积Q */
        for (int i = 0; i < n; ++i) {
            double dot = Q[i][k + 1];
            for (int j = 1; j < v.size(); ++j)
                dot += v[j] * Q[i][k + 1 + j];
            Q[i][k + 1] -= beta * dot;
            for (int j = 1; j < v.size(); ++j)
                Q[i][k + 1 + j] -= beta * v[j] * dot;
        }
    }
}

/* ========== Francis双移QR步 ========== */

void SchurDecomp2::francisDoubleShift(
    QVector<QVector<double>>& H, QVector<QVector<double>>& Q,
    int p, int q) const
{
    int n = H.size();
    if (q - p < 2) return;

    /* 从2x2尾部块计算隐式双移 */
    double a = H[q - 2][q - 2];
    double b = H[q - 2][q - 1];
    double c = H[q - 1][q - 2];
    double d = H[q - 1][q - 1];

    double trace = a + d;       /* 迹 */
    double det = a * d - b * c; /* 行列式 */

    /* 计算第一列: x = H^2 - trace*H + det*I 的第一列 */
    double x = H[p][p] * H[p][p] + H[p][p + 1] * H[p + 1][p] - trace * H[p][p] + det;
    double y = H[p + 1][p] * (H[p][p] + H[p + 1][p + 1] - trace);
    double z = H[p + 1][p] * H[p + 2][p + 1];

    /* 使用Givens旋转追赶(bulge chasing) */
    for (int k = p; k < q - 2; ++k) {
        /* 计算Givens旋转消除y和z */
        double r = qSqrt(x * x + y * y + z * z);
        if (r < 1e-15) { x = 0; y = 0; z = 0; continue; }

        double c1 = x / r;
        double s1 = y / r;
        /* 三向量旋转, 简化为只旋转前两个分量 */
        double cs = qSqrt(c1 * c1 + s1 * s1);
        if (cs < 1e-15) cs = 1.0;
        c1 /= cs;
        s1 /= cs;

        int colStart = qMax(0, k - 1);
        int rowEnd = qMin(k + 4, n);

        /* 左乘Givens */
        for (int j = colStart; j < n; ++j) {
            double h1 = H[k][j];
            double h2 = H[k + 1][j];
            H[k][j]     = c1 * h1 + s1 * h2;
            H[k + 1][j] = -s1 * h1 + c1 * h2;
        }

        /* 右乘Givens */
        for (int i = 0; i < rowEnd; ++i) {
            double h1 = H[i][k];
            double h2 = H[i][k + 1];
            H[i][k]     = c1 * h1 + s1 * h2;
            H[i][k + 1] = -s1 * h1 + c1 * h2;
        }

        /* 累积到Q */
        for (int i = 0; i < n; ++i) {
            double q1 = Q[i][k];
            double q2 = Q[i][k + 1];
            Q[i][k]     = c1 * q1 + s1 * q2;
            Q[i][k + 1] = -s1 * q1 + c1 * q2;
        }

        /* 更新追赶变量 */
        x = H[k + 1][k];
        if (k + 2 < q) {
            y = H[k + 2][k];
            if (k + 3 < q) z = H[k + 3][k]; else z = 0.0;
        } else {
            y = 0.0; z = 0.0;
        }
    }

    /* 最后一个2x2旋转 */
    if (q - 2 >= p) {
        int k = q - 2;
        double h1 = H[k][k];
        double h2 = H[k + 1][k];
        double r = qSqrt(h1 * h1 + h2 * h2);
        if (r > 1e-15) {
            double c1 = h1 / r;
            double s1 = h2 / r;

            for (int j = k; j < n; ++j) {
                double a1 = H[k][j];
                double a2 = H[k + 1][j];
                H[k][j]     = c1 * a1 + s1 * a2;
                H[k + 1][j] = -s1 * a1 + c1 * a2;
            }
            for (int i = 0; i < q; ++i) {
                double a1 = H[i][k];
                double a2 = H[i][k + 1];
                H[i][k]     = c1 * a1 + s1 * a2;
                H[i][k + 1] = -s1 * a1 + c1 * a2;
            }
            for (int i = 0; i < n; ++i) {
                double q1 = Q[i][k];
                double q2 = Q[i][k + 1];
                Q[i][k]     = c1 * q1 + s1 * q2;
                Q[i][k + 1] = -s1 * q1 + c1 * q2;
            }
        }
    }
}

/* ========== 特征值提取 ========== */

QVector<SchurDecomp2::ComplexEigen> SchurDecomp2::extractEigenvalues(
    const QVector<QVector<double>>& T) const
{
    QVector<ComplexEigen> eigenvalues;
    int n = T.size();
    int i = 0;
    while (i < n) {
        if (i == n - 1 || qAbs(T[i + 1][i]) <= m_epsilon) {
            /* 1x1块 → 实特征值 */
            eigenvalues.append({T[i][i], 0.0});
            ++i;
        } else {
            /* 2x2块 → 可能共轭复特征值 */
            double a = T[i][i];
            double b = T[i][i + 1];
            double c = T[i + 1][i];
            double d = T[i + 1][i + 1];
            double trace = a + d;
            double det = a * d - b * c;
            double disc = trace * trace - 4.0 * det;

            if (disc >= 0.0) {
                double sqrtDisc = qSqrt(disc);
                eigenvalues.append({(trace + sqrtDisc) / 2.0, 0.0});
                eigenvalues.append({(trace - sqrtDisc) / 2.0, 0.0});
            } else {
                double sqrtDisc = qSqrt(-disc);
                eigenvalues.append({trace / 2.0,  sqrtDisc / 2.0});
                eigenvalues.append({trace / 2.0, -sqrtDisc / 2.0});
            }
            i += 2;
        }
    }
    return eigenvalues;
}

/* ========== 重置统计 ========== */

void SchurDecomp2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
