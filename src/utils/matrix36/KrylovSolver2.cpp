/**
 * @file KrylovSolver2.cpp
 * @brief Krylov子空间求解器增强实现 — BiCGSTAB/CGS迭代法/COO转CSR
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix36/KrylovSolver2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <numeric>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
KrylovSolver2::KrylovSolver2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("KrylovSolver2"));
}

/**
 * @brief 从COO(坐标)格式构建CSR(压缩稀疏行)矩阵
 *
 * COO格式: (rows[i], cols[i], vals[i]) 描述每个非零元素。
 * 转换为CSR格式以支持高效的稀疏矩阵-向量乘法。
 *
 * @param rows 行索引数组
 * @param cols 列索引数组
 * @param vals 非零值数组
 * @param n 矩阵维度
 */
void KrylovSolver2::buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                                  const QVector<double>& vals, int n)
{
    m_n = n;
    int nnz = vals.size();

    /* 按行排序COO三元组 */
    QVector<int> indices(nnz);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        if (rows[a] != rows[b]) return rows[a] < rows[b];
        return cols[a] < cols[b];
    });

    /* 构建CSR */
    m_rowPtr.assign(n + 1, 0);
    m_colIdx.resize(nnz);
    m_values.resize(nnz);

    for (int i = 0; i < nnz; ++i) {
        m_colIdx[i] = cols[indices[i]];
        m_values[i] = vals[indices[i]];
        m_rowPtr[rows[indices[i]] + 1]++;
    }

    /* 前缀和得到行指针 */
    for (int i = 0; i < n; ++i) {
        m_rowPtr[i + 1] += m_rowPtr[i];
    }
}

/**
 * @brief 设置收敛容差
 * @param tol 相对残差容差
 */
void KrylovSolver2::setTolerance(double tol)
{
    m_tol = qMax(1e-15, tol);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代数
 */
void KrylovSolver2::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 稀疏矩阵-向量乘法 y = A * x
 * @param x 输入向量
 * @return 输出向量
 */
QVector<double> KrylovSolver2::spmv(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
            sum += m_values[j] * x[m_colIdx[j]];
        }
        y[i] = sum;
    }
    return y;
}

/**
 * @brief 共轭梯度法(CG) — 仅适用于对称正定矩阵
 *
 * @param rhs 右端向量
 * @return 解向量
 */
QVector<double> KrylovSolver2::solveCG(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_n, 0.0);
    if (m_n == 0) return x;

    QVector<double> r = rhs; /* r = b - A*x = b (x=0) */
    QVector<double> p = r;
    double rsOld = dot(r, r);

    double bNorm = qSqrt(dot(rhs, rhs));
    if (bNorm < 1e-15) {
        m_lastIter = 0;
        return x;
    }

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<double> Ap = spmv(p);
        double pAp = dot(p, Ap);

        if (qFabs(pAp) < 1e-30) break;

        double alpha = rsOld / pAp;

        for (int i = 0; i < m_n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        double rsNew = dot(r, r);

        if (qSqrt(rsNew) / bNorm < m_tol) {
            m_lastIter = iter + 1;
            break;
        }

        double beta = rsNew / rsOld;
        for (int i = 0; i < m_n; ++i) {
            p[i] = r[i] + beta * p[i];
        }

        rsOld = rsNew;
        m_lastIter = iter + 1;
    }

    updateStats(timer.elapsed());
    emit solveComplete(true, m_lastIter);
    return x;
}

/**
 * @brief BiCGSTAB(双共轭梯度稳定化)迭代求解器
 *
 * 适用于一般非对称矩阵。比标准BiCG更平滑的收敛行为。
 * 使用r_tilde = r0作为影子残差（选择策略）。
 *
 * @param rhs 右端向量
 * @return 解向量
 */
QVector<double> KrylovSolver2::solveBiCGSTAB(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_n, 0.0);
    if (m_n == 0) return x;

    QVector<double> r = rhs; /* r0 = b - A*x0 */
    QVector<double> rTilde = r; /* r_tilde = r0，影子残差 */

    double bNorm = qSqrt(dot(rhs, rhs));
    if (bNorm < 1e-15) {
        m_lastIter = 0;
        updateStats(timer.elapsed());
        return x;
    }

    double rhoPrev = 1.0;
    double omega = 1.0;
    QVector<double> p(m_n, 0.0);
    QVector<double> v(m_n, 0.0);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* rho = (r_tilde, r) */
        double rho = dot(rTilde, r);
        if (qFabs(rho) < 1e-30) break;

        double beta = (rho / rhoPrev) * (1.0 / omega);

        /* p = r + beta * (p - omega * v) */
        for (int i = 0; i < m_n; ++i) {
            p[i] = r[i] + beta * (p[i] - omega * v[i]);
        }

        /* v = A * p */
        v = spmv(p);

        /* alpha = rho / (r_tilde, v) */
        double rTv = dot(rTilde, v);
        if (qFabs(rTv) < 1e-30) break;
        double alpha = rho / rTv;

        /* s = r - alpha * v */
        QVector<double> s(m_n);
        for (int i = 0; i < m_n; ++i) {
            s[i] = r[i] - alpha * v[i];
        }

        /* t = A * s */
        QVector<double> t = spmv(s);

        /* omega = (t, s) / (t, t) */
        double tTs = dot(t, s);
        double tTt = dot(t, t);
        if (qFabs(tTt) < 1e-30) break;
        omega = tTs / tTt;

        /* x = x + alpha * p + omega * s */
        for (int i = 0; i < m_n; ++i) {
            x[i] += alpha * p[i] + omega * s[i];
            r[i] = s[i] - omega * t[i];
        }

        m_lastIter = iter + 1;

        /* 检查收敛 */
        double rNorm = qSqrt(dot(r, r));
        if (rNorm / bNorm < m_tol) break;

        rhoPrev = rho;
    }

    updateStats(timer.elapsed());
    emit solveComplete(true, m_lastIter);
    return x;
}

/**
 * @brief CGS(共轭梯度平方)迭代求解器
 *
 * 将BiCG的多项式残差平方化，通常比BiCG收敛更快但可能震荡。
 * 不需要A^T的转置运算。
 *
 * @param rhs 右端向量
 * @return 解向量
 */
QVector<double> KrylovSolver2::solveCGS(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_n, 0.0);
    if (m_n == 0) return x;

    QVector<double> r = rhs;
    QVector<double> rTilde = r;

    double bNorm = qSqrt(dot(rhs, rhs));
    if (bNorm < 1e-15) {
        m_lastIter = 0;
        updateStats(timer.elapsed());
        return x;
    }

    double rhoPrev = 1.0;
    QVector<double> p(m_n, 0.0);
    QVector<double> q(m_n, 0.0);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        double rho = dot(rTilde, r);
        if (qFabs(rho) < 1e-30) break;

        double beta = rho / rhoPrev;

        /* u = r + beta * q */
        QVector<double> u(m_n);
        for (int i = 0; i < m_n; ++i) {
            u[i] = r[i] + beta * q[i];
        }

        /* p = u + beta * (q + beta * p) */
        for (int i = 0; i < m_n; ++i) {
            p[i] = u[i] + beta * (q[i] + beta * p[i]);
        }

        /* v = A * p */
        QVector<double> v = spmv(p);

        double sigma = dot(rTilde, v);
        if (qFabs(sigma) < 1e-30) break;
        double alpha = rho / sigma;

        /* q = u - alpha * v */
        for (int i = 0; i < m_n; ++i) {
            q[i] = u[i] - alpha * v[i];
        }

        /* x = x + alpha * (u + q) */
        for (int i = 0; i < m_n; ++i) {
            x[i] += alpha * (u[i] + q[i]);
        }

        /* r = A * (u + q) 再减去... 直接用残差更新 */
        QVector<double> uq(m_n);
        for (int i = 0; i < m_n; ++i) {
            uq[i] = u[i] + q[i];
        }
        QVector<double> t = spmv(uq);
        for (int i = 0; i < m_n; ++i) {
            r[i] -= alpha * t[i];
        }

        m_lastIter = iter + 1;

        double rNorm = qSqrt(dot(r, r));
        if (rNorm / bNorm < m_tol) break;

        rhoPrev = rho;
    }

    updateStats(timer.elapsed());
    emit solveComplete(true, m_lastIter);
    return x;
}

/**
 * @brief 获取上次求解的迭代次数
 * @return 迭代次数
 */
int KrylovSolver2::lastIterations() const
{
    return m_lastIter;
}

/**
 * @brief 向量点积辅助函数
 */
double KrylovSolver2::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    const int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

/**
 * @brief 更新统计信息
 * @param elapsedMs 本次耗时(ms)
 */
void KrylovSolver2::updateStats(double elapsedMs)
{
    m_timeSum += elapsedMs;
    m_stats.totalSolves++;
    m_stats.totalIterations += m_lastIter;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
}

/**
 * @brief 重置所有累积统计信息
 */
void KrylovSolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
