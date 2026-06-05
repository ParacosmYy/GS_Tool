/**
 * @file SparseBiCGSTAB.cpp
 * @brief BiCGSTAB迭代求解器实现 - 稀疏线性方程组求解
 *
 * 双共轭梯度稳定法(Bi-Conjugate Gradient Stabilized)用于求解
 * 非对称稀疏线性方程组 Ax = b。采用CSR格式存储稀疏矩阵，
 * 支持预处理和收敛监测。
 */

#include "utils/matrix40/SparseBiCGSTAB.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
SparseBiCGSTAB::SparseBiCGSTAB(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 从COO(坐标)格式构建CSR(压缩行存储)格式
 *
 * 将(row, col, val)三元组转换为CSR格式:
 * - m_rowPtr: 每行起始位置指针(长度n+1)
 * - m_colIdx: 列索引数组
 * - m_values: 非零值数组
 *
 * @param rows 行索引数组
 * @param cols 列索引数组
 * @param vals 值数组
 * @param n 矩阵维度
 */
void SparseBiCGSTAB::buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                                   const QVector<double>& vals, int n)
{
    m_n = n;
    const int nnz = qMin(qMin(rows.size(), cols.size()), vals.size());

    /* 统计每行非零元素数 */
    QVector<int> rowCounts(n + 1, 0);
    for (int i = 0; i < nnz; ++i) {
        if (rows[i] >= 0 && rows[i] < n && cols[i] >= 0 && cols[i] < n)
            rowCounts[rows[i] + 1]++;
    }

    /* 构建行指针(前缀和) */
    m_rowPtr.resize(n + 1, 0);
    for (int i = 0; i < n; ++i)
        m_rowPtr[i + 1] = m_rowPtr[i] + rowCounts[i + 1];

    /* 填充列索引和值 */
    m_colIdx.resize(m_rowPtr[n], 0);
    m_values.resize(m_rowPtr[n], 0.0);

    QVector<int> pos = m_rowPtr;
    for (int i = 0; i < nnz; ++i) {
        if (rows[i] >= 0 && rows[i] < n && cols[i] >= 0 && cols[i] < n) {
            int dest = pos[rows[i]]++;
            m_colIdx[dest] = cols[i];
            m_values[dest] = vals[i];
        }
    }

    /* 按行内列索引排序 */
    for (int r = 0; r < n; ++r) {
        int start = m_rowPtr[r];
        int end = m_rowPtr[r + 1];
        /* 插入排序(短行更快) */
        for (int i = start + 1; i < end; ++i) {
            int c = m_colIdx[i];
            double v = m_values[i];
            int j = i - 1;
            while (j >= start && m_colIdx[j] > c) {
                m_colIdx[j + 1] = m_colIdx[j];
                m_values[j + 1] = m_values[j];
                j--;
            }
            m_colIdx[j + 1] = c;
            m_values[j + 1] = v;
        }
    }
}

/**
 * @brief 设置收敛容差
 * @param tol 残差范数容差
 */
void SparseBiCGSTAB::setTolerance(double tol)
{
    m_tol = qMax(1e-15, tol);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代次数
 */
void SparseBiCGSTAB::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief CSR稀疏矩阵-向量乘法 y = A * x
 * @param x 输入向量
 * @return 乘积结果
 */
QVector<double> SparseBiCGSTAB::spmv(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int r = 0; r < m_n; ++r) {
        double sum = 0.0;
        for (int idx = m_rowPtr[r]; idx < m_rowPtr[r + 1]; ++idx) {
            sum += m_values[idx] * x[m_colIdx[idx]];
        }
        y[r] = sum;
    }
    return y;
}

/**
 * @brief 向量点积
 */
static double dotProduct(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i)
        sum += a[i] * b[i];
    return sum;
}

/**
 * @brief 向量范数
 */
static double norm2(const QVector<double>& v)
{
    return qSqrt(dotProduct(v, v));
}

/**
 * @brief 求解稀疏线性方程组 Ax = b
 *
 * BiCGSTAB算法流程:
 * 1. 初始化: r0 = b - Ax0, 选取r0_hat = r0
 * 2. 迭代:
 *    a. rho = r0_hat . r
 *    b. 如果rho接近零则失败
 *    c. 如果第一次: p = r; 否则更新p
 *    d. v = A * p
 *    e. alpha = rho / (r0_hat . v)
 *    f. s = r - alpha * v
 *    g. t = A * s
 *    h. omega = (t . s) / (t . t)
 *    i. x = x + alpha * p + omega * s
 *    j. r = s - omega * t
 * 3. 收敛检查: ||r|| < tol * ||b||
 *
 * @param rhs 右端向量b
 * @return 解向量x
 */
QVector<double> SparseBiCGSTAB::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n <= 0 || rhs.size() < m_n) {
        m_stats.totalSolves++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solveComplete(false, 0);
        return QVector<double>(m_n, 0.0);
    }

    /* 初始猜测x0 = 0 */
    QVector<double> x(m_n, 0.0);

    /* r = b - A*x = b */
    QVector<double> r = rhs;

    /* 残差向量范数 */
    double bnorm = norm2(rhs);
    if (bnorm < 1e-30) {
        /* b = 0 则 x = 0 */
        m_stats.totalSolves++;
        m_lastIter = 0;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solveComplete(true, 0);
        return x;
    }

    /* 选取r0_hat = r */
    QVector<double> r0hat = r;

    QVector<double> p(m_n, 0.0);
    QVector<double> v(m_n, 0.0);
    QVector<double> s(m_n, 0.0);
    QVector<double> t(m_n, 0.0);

    double rho = 1.0, rhoPrev = 1.0;
    double alpha = 1.0, omega = 1.0;
    bool converged = false;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        m_lastIter = iter + 1;

        /* rho = r0_hat . r */
        rho = dotProduct(r0hat, r);

        /* 检查rho是否接近零(算法失败) */
        if (qAbs(rho) < 1e-30) break;

        /* 更新搜索方向p */
        if (iter == 0) {
            p = r;
        } else {
            double beta = (rho / rhoPrev) * (alpha / omega);
            /* p = r + beta * (p - omega * v) */
            for (int i = 0; i < m_n; ++i)
                p[i] = r[i] + beta * (p[i] - omega * v[i]);
        }

        /* v = A * p */
        v = spmv(p);

        /* alpha = rho / (r0_hat . v) */
        double r0hatDotV = dotProduct(r0hat, v);
        if (qAbs(r0hatDotV) < 1e-30) break;
        alpha = rho / r0hatDotV;

        /* s = r - alpha * v */
        for (int i = 0; i < m_n; ++i)
            s[i] = r[i] - alpha * v[i];

        /* 检查s的范数是否足够小(提前收敛) */
        double snorm = norm2(s);
        if (snorm / bnorm < m_tol) {
            for (int i = 0; i < m_n; ++i)
                x[i] += alpha * p[i];
            converged = true;
            break;
        }

        /* t = A * s */
        t = spmv(s);

        /* omega = (t . s) / (t . t) */
        double tDotT = dotProduct(t, t);
        if (tDotT < 1e-30) break;
        omega = dotProduct(t, s) / tDotT;

        /* 更新解: x += alpha * p + omega * s */
        for (int i = 0; i < m_n; ++i)
            x[i] += alpha * p[i] + omega * s[i];

        /* 更新残差: r = s - omega * t */
        for (int i = 0; i < m_n; ++i)
            r[i] = s[i] - omega * t[i];

        rhoPrev = rho;

        /* 收敛检查 */
        double rnorm = norm2(r);
        if (rnorm / bnorm < m_tol) {
            converged = true;
            break;
        }
    }

    m_stats.totalSolves++;
    m_stats.totalIterations += m_lastIter;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveComplete(converged, m_lastIter);
    return x;
}

/**
 * @brief 获取上次求解的迭代次数
 * @return 上次求解使用的迭代数
 */
int SparseBiCGSTAB::lastIterations() const
{
    return m_lastIter;
}

/**
 * @brief 重置所有统计数据
 */
void SparseBiCGSTAB::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
