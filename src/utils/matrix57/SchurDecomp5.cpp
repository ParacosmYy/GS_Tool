/**
 * @file SchurDecomp5.cpp
 * @brief Schur分解实现，使用Francis双移QR迭代算法
 *
 * 实现了实矩阵的Schur分解 A = Q * T * Q^T，其中T为上三角矩阵（实Schur形式），
 * Q为正交矩阵。对角块为1x1（实特征值）或2x2（复共轭特征值对）。
 * 适用于特征值计算和矩阵分析。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix57/SchurDecomp5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化空矩阵
 * @param parent 父QObject对象指针
 */
SchurDecomp5::SchurDecomp5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 获取实Schur形式的特征值
 *
 * 从上三角矩阵T的对角块中提取特征值：
 * - 1x1对角块：实特征值
 * - 2x2对角块：复共轭特征值对
 *
 * @return 特征值列表（复数的实部和虚部对）
 */
QVector<QPair<double, double>> SchurDecomp5::eigenvalues() const
{
    QVector<QPair<double, double>> eigs;
    if (m_n == 0) return eigs;

    int i = 0;
    while (i < m_n) {
        if (i == m_n - 1) {
            /* 1x1块：实特征值 */
            eigs.append({m_T[i * m_n + i], 0.0});
            ++i;
        } else {
            /* 检查是否为2x2块 */
            double a11 = m_T[i * m_n + i];
            double a12 = m_T[i * m_n + i + 1];
            double a21 = m_T[(i + 1) * m_n + i];
            double a22 = m_T[(i + 1) * m_n + i + 1];

            if (qAbs(a21) < 1e-12) {
                /* 两个1x1块 */
                eigs.append({a11, 0.0});
                eigs.append({a22, 0.0});
                i += 2;
            } else {
                /* 2x2块：复共轭特征值 */
                double tr = a11 + a22;
                double det = a11 * a22 - a12 * a21;
                double disc = tr * tr - 4.0 * det;
                double realPart = tr / 2.0;
                double imagPart = qSqrt(qMax(0.0, -disc)) / 2.0;
                eigs.append({realPart, imagPart});
                eigs.append({realPart, -imagPart});
                i += 2;
            }
        }
    }
    return eigs;
}

/**
 * @brief 执行Schur分解
 *
 * 算法流程：
 * 1. 将输入矩阵A复制到T中
 * 2. 初始化Q为单位矩阵
 * 3. 对T进行Hessenberg化简
 * 4. 对Hessenberg矩阵执行Francis QR迭代
 * 5. 得到上三角Schur形式T和正交矩阵Q
 *
 * @param A 输入矩阵，以行主序一维数组存储（大小为n*n）
 * @param n 矩阵维度
 * @return true分解成功，false分解失败
 */
bool SchurDecomp5::decompose(const QVector<double>& A, int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || A.size() < n * n) return false;

    m_n = n;
    m_T = A;
    m_Q.resize(n * n, 0.0);

    /* 初始化Q为单位矩阵 */
    for (int i = 0; i < n; ++i)
        m_Q[i * n + i] = 1.0;

    /* Step 1: Hessenberg化简（Householder变换） */
    for (int k = 0; k < n - 2; ++k) {
        /* 计算Householder向量 */
        double norm = 0.0;
        for (int i = k + 2; i < n; ++i)
            norm += m_T[i * n + k] * m_T[i * n + k];
        norm = qSqrt(norm + m_T[(k + 1) * n + k] * m_T[(k + 1) * n + k]);

        if (norm < 1e-15) continue;

        double alpha = (m_T[(k + 1) * n + k] > 0) ? -norm : norm;
        double r = qSqrt(2.0 * alpha * (alpha - m_T[(k + 1) * n + k]));

        if (qAbs(r) < 1e-15) continue;

        QVector<double> v(n, 0.0);
        v[k + 1] = (m_T[(k + 1) * n + k] - alpha) / r;
        for (int i = k + 2; i < n; ++i)
            v[i] = m_T[i * n + k] / r;

        /* 应用 Householder: T = (I - 2vv^T) T (I - 2vv^T) */
        /* 左乘: T -= 2 * v * (v^T * T) */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = k + 1; i < n; ++i)
                dot += v[i] * m_T[i * n + j];
            for (int i = k + 1; i < n; ++i)
                m_T[i * n + j] -= 2.0 * v[i] * dot;
        }

        /* 右乘: T -= 2 * (T * v) * v^T */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j)
                dot += m_T[i * n + j] * v[j];
            for (int j = k + 1; j < n; ++j)
                m_T[i * n + j] -= 2.0 * dot * v[j];
        }

        /* 更新Q: Q = Q * (I - 2vv^T) */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j)
                dot += m_Q[i * n + j] * v[j];
            for (int j = k + 1; j < n; ++j)
                m_Q[i * n + j] -= 2.0 * dot * v[j];
        }
    }

    /* Step 2: Francis QR迭代 */
    francisQR(m_T, m_Q, n);

    /* 更新统计 */
    m_stats.totalDecompositions++;
    m_stats.matrixSize = n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n);
    return true;
}

/**
 * @brief Francis双移QR迭代
 *
 * 对Hessenberg矩阵执行Francis双隐移QR迭代。
 * 通过双移策略处理复共轭特征值对，避免复数运算。
 * 使用Wilkinson位移提高收敛速度。
 *
 * @param H Hessenberg矩阵（会被修改为上三角Schur形式）
 * @param Q 累积正交变换矩阵
 * @param n 矩阵维度
 */
void SchurDecomp5::francisQR(QVector<double>& H, QVector<double>& Q, int n)
{
    const int maxIter = 100 * n;
    int p = n - 1;

    for (int iter = 0; iter < maxIter && p > 0; ++iter) {
        /* 寻找最小未收敛子矩阵 [0..p] */
        int q = p;
        while (q > 0 && qAbs(H[q * n + q - 1]) > 1e-10 * (qAbs(H[(q - 1) * n + q - 1]) + qAbs(H[q * n + q])))
            --q;

        if (q == p) {
            /* 1x1块已收敛 */
            --p;
            continue;
        }

        if (q == p - 1) {
            /* 2x2块，检查是否需要交换 */
            if (qAbs(H[p * n + p - 1]) <= 1e-10 * (qAbs(H[(p - 1) * n + p - 1]) + qAbs(H[p * n + p]))) {
                H[p * n + p - 1] = 0.0;
                p -= 2;
                continue;
            }
        }

        /* Wilkinson位移 */
        double s = H[(p - 1) * n + p - 1] + H[p * n + p];
        double t = H[(p - 1) * n + p - 1] * H[p * n + p] - H[(p - 1) * n + p] * H[p * n + p - 1];

        /* 计算第一列的隐式双移 */
        double x = H[q * n + q] * H[q * n + q] + H[q * n + q + 1] * H[(q + 1) * n + q] - s * H[q * n + q] + t;
        double y = H[(q + 1) * n + q] * (H[q * n + q] + H[(q + 1) * n + q + 1] - s);
        double z = H[(q + 1) * n + q] * H[(q + 2) * n + q + 1];

        /* 执行隐式双移QR步 */
        for (int k = q; k < p - 1; ++k) {
            /* 构造并应用Givens旋转 */
            double r = qSqrt(x * x + y * y + z * z);
            if (r < 1e-30) break;

            double c1 = x / r, c2 = y / r, c3 = z / r;

            int colStart = qMax(0, k - 1);
            int colEnd = qMin(n - 1, k + 3);

            /* 左乘旋转 */
            for (int j = colStart; j <= colEnd; ++j) {
                double t1 = H[k * n + j];
                double t2 = H[(k + 1) * n + j];
                double t3 = H[(k + 2) * n + j];
                H[k * n + j] = c1 * t1 + c2 * t2 + c3 * t3;
                H[(k + 1) * n + j] = c2 * t1 - c1 * t2;
                H[(k + 2) * n + j] = c3 * t1 - c1 * t3;
            }

            /* 右乘旋转 */
            int rowEnd = qMin(n - 1, k + 3);
            for (int i = 0; i <= rowEnd; ++i) {
                double t1 = H[i * n + k];
                double t2 = H[i * n + k + 1];
                double t3 = H[i * n + k + 2];
                H[i * n + k] = c1 * t1 + c2 * t2 + c3 * t3;
                H[i * n + k + 1] = c2 * t1 - c1 * t2;
                H[i * n + k + 2] = c3 * t1 - c1 * t3;
            }

            /* 更新Q矩阵 */
            for (int i = 0; i < n; ++i) {
                double t1 = Q[i * n + k];
                double t2 = Q[i * n + k + 1];
                double t3 = Q[i * n + k + 2];
                Q[i * n + k] = c1 * t1 + c2 * t2 + c3 * t3;
                Q[i * n + k + 1] = c2 * t1 - c1 * t2;
                Q[i * n + k + 2] = c3 * t1 - c1 * t3;
            }

            /* 更新x, y, z为下一次迭代 */
            x = H[(k + 1) * n + k];
            y = H[(k + 2) * n + k];
            if (k < p - 2)
                z = H[(k + 3) * n + k];
        }

        /* 清理微小的次对角元素 */
        if (qAbs(H[(q + 1) * n + q]) <= 1e-10)
            H[(q + 1) * n + q] = 0.0;
    }
}

/**
 * @brief 重置所有统计数据
 */
void SchurDecomp5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
