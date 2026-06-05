/**
 * @file SparseBiCGSTAB2.cpp
 * @brief 稀疏BiCGSTAB2实现 — 预条件+右端GMRES稳定化
 *
 * 双共轭梯度稳定法(BiCGSTAB)的稀疏矩阵求解器实现。
 * 相比BiCG具有更好的数值稳定性，支持对角预条件。
 * 适用于大型稀疏非对称线性方程组。
 */

#include "utils/matrix49/SparseBiCGSTAB2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
SparseBiCGSTAB2::SparseBiCGSTAB2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置稀疏矩阵(CSR格式)
 * @param n 矩阵维度
 * @param rowPtr 行偏移数组，大小为n+1
 * @param colIdx 列索引数组
 * @param values 非零元素值数组
 */
void SparseBiCGSTAB2::setMatrix(int n, const QVector<int>& rowPtr,
                                  const QVector<int>& colIdx,
                                  const QVector<double>& values)
{
    m_n = n;
    m_rowPtr = rowPtr;
    m_colIdx = colIdx;
    m_values = values;
    m_precondBuilt = false;
}

/**
 * @brief 设置收敛容差
 * @param tol 相对残差容差
 */
void SparseBiCGSTAB2::setTolerance(double tol)
{
    m_tol = qMax(1e-15, tol);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代次数
 */
void SparseBiCGSTAB2::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 构建对角预条件子
 * @return true成功，false矩阵未设置
 *
 * 使用Jacobi对角预条件: M = diag(A)
 */
bool SparseBiCGSTAB2::buildPreconditioner()
{
    if (m_n == 0 || m_rowPtr.isEmpty()) return false;

    m_diagPrecond.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        double diag = 1.0;
        for (int idx = m_rowPtr[i]; idx < m_rowPtr[i + 1]; ++idx) {
            if (m_colIdx[idx] == i) {
                diag = m_values[idx];
                break;
            }
        }
        m_diagPrecond[i] = (qFabs(diag) > 1e-15) ? 1.0 / diag : 1.0;
    }

    m_precondBuilt = true;
    return true;
}

/**
 * @brief 求解线性方程组 Ax = b
 * @param rhs 右端向量b
 * @return 解向量x
 *
 * BiCGSTAB算法:
 * 1. 计算初始残差 r0 = b - Ax0
 * 2. 选择r0^使得(r0^, r0) != 0
 * 3. 迭代: rho -> beta -> p -> phat -> v -> alpha -> s -> shat -> t -> omega -> x更新
 * 4. 收敛判断: ||s|| 和 ||r|| 小于容差
 */
QVector<double> SparseBiCGSTAB2::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || rhs.size() != m_n) {
        return QVector<double>(m_n, 0.0);
    }

    QVector<double> x(m_n, 0.0);

    /* 初始残差 r = b - A*x (x0 = 0, 所以 r = b) */
    QVector<double> r = rhs;
    QVector<double> rHat = r; /* r0^ = r0 */

    double bNorm = 0.0;
    for (int i = 0; i < m_n; ++i) bNorm += rhs[i] * rhs[i];
    bNorm = qSqrt(bNorm);
    if (bNorm < 1e-15) {
        m_iterUsed = 0;
        m_resNorm = 0.0;
        return x;
    }

    /* 初始化迭代变量 */
    QVector<double> p(m_n, 0.0);
    QVector<double> v(m_n, 0.0);
    QVector<double> s(m_n, 0.0);
    QVector<double> t(m_n, 0.0);

    double rhoPrev = 1.0;
    double alpha = 1.0;
    double omega = 1.0;
    int iter = 0;
    int breakdowns = 0;
    bool converged = false;

    for (iter = 0; iter < m_maxIter; ++iter) {
        /* ---- rho = (rHat, r) ---- */
        double rho = 0.0;
        for (int i = 0; i < m_n; ++i) {
            rho += rHat[i] * r[i];
        }

        if (qFabs(rho) < 1e-30) {
            /* Breakdown: 重新初始化 */
            breakdowns++;
            r = rhs;
            /* r = b - Ax */
            QVector<double> Ax = spmv(x);
            for (int i = 0; i < m_n; ++i) r[i] -= Ax[i];
            rHat = r;
            rho = 1.0;
            p.fill(0.0);
            v.fill(0.0);
            rhoPrev = 1.0;
            alpha = 1.0;
            omega = 1.0;
            continue;
        }

        /* ---- beta = (rho / rhoPrev) * (alpha / omega) ---- */
        double beta = (rho / rhoPrev) * (alpha / omega);
        rhoPrev = rho;

        /* ---- p = r + beta * (p - omega * v) ---- */
        for (int i = 0; i < m_n; ++i) {
            p[i] = r[i] + beta * (p[i] - omega * v[i]);
        }

        /* ---- phat = M^{-1} * p ---- */
        QVector<double> pHat = precondSolve(p);

        /* ---- v = A * phat ---- */
        v = spmv(pHat);

        /* ---- alpha = rho / (rHat, v) ---- */
        double rHatDotV = 0.0;
        for (int i = 0; i < m_n; ++i) {
            rHatDotV += rHat[i] * v[i];
        }
        alpha = rho / ((qFabs(rHatDotV) > 1e-30) ? rHatDotV : 1e-30);

        /* ---- s = r - alpha * v ---- */
        for (int i = 0; i < m_n; ++i) {
            s[i] = r[i] - alpha * v[i];
        }

        /* 检查s范数 */
        double sNorm = 0.0;
        for (int i = 0; i < m_n; ++i) sNorm += s[i] * s[i];
        sNorm = qSqrt(sNorm);

        if (sNorm / bNorm < m_tol) {
            /* s足够小，更新x后退出 */
            for (int i = 0; i < m_n; ++i) {
                x[i] += alpha * pHat[i];
            }
            m_resNorm = sNorm;
            converged = true;
            break;
        }

        /* ---- shat = M^{-1} * s ---- */
        QVector<double> sHat = precondSolve(s);

        /* ---- t = A * shat ---- */
        t = spmv(sHat);

        /* ---- omega = (t, s) / (t, t) ---- */
        double tDotS = 0.0;
        double tDotT = 0.0;
        for (int i = 0; i < m_n; ++i) {
            tDotS += t[i] * s[i];
            tDotT += t[i] * t[i];
        }
        omega = (qFabs(tDotT) > 1e-30) ? tDotS / tDotT : 0.0;

        /* ---- x = x + alpha * phat + omega * shat ---- */
        for (int i = 0; i < m_n; ++i) {
            x[i] += alpha * pHat[i] + omega * sHat[i];
        }

        /* ---- r = s - omega * t ---- */
        for (int i = 0; i < m_n; ++i) {
            r[i] = s[i] - omega * t[i];
        }

        /* 收敛检查 */
        double rNorm = 0.0;
        for (int i = 0; i < m_n; ++i) rNorm += r[i] * r[i];
        rNorm = qSqrt(rNorm);
        m_resNorm = rNorm;

        if (rNorm / bNorm < m_tol) {
            converged = true;
            break;
        }
    }

    m_iterUsed = iter;

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolves++;
    m_stats.totalIterations += iter;
    m_stats.totalBreakdowns += breakdowns;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(iter, m_resNorm, converged);
    return x;
}

/**
 * @brief 稀疏矩阵向量乘法 y = Ax
 * @param x 输入向量
 * @return 乘积向量
 */
QVector<double> SparseBiCGSTAB2::spmv(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int idx = m_rowPtr[i]; idx < m_rowPtr[i + 1]; ++idx) {
            int col = m_colIdx[idx];
            if (col >= 0 && col < x.size()) {
                sum += m_values[idx] * x[col];
            }
        }
        y[i] = sum;
    }
    return y;
}

/**
 * @brief 对角预条件求解 Mz = r
 * @param r 输入向量
 * @return 预条件后的向量
 */
QVector<double> SparseBiCGSTAB2::precondSolve(const QVector<double>& r) const
{
    if (!m_precondBuilt) return r;

    QVector<double> z(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        z[i] = r[i] * m_diagPrecond[i];
    }
    return z;
}

/**
 * @brief 重置所有统计信息
 */
void SparseBiCGSTAB2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
