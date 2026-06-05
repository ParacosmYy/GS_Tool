/**
 * @file SparseSolver.cpp
 * @brief 稀疏线性求解器实现 — 预条件共轭梯度法
 *
 * 使用CSR格式存储稀疏矩阵，支持Jacobi和SSOR两种预条件器，
 * 共轭梯度迭代求解对称正定线性方程组 Ax = b。
 */

#include "utils/matrix6/SparseSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ── 构造函数 ──

/** @brief 构造函数 @param parent 父对象 */
SparseSolver::SparseSolver(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SparseSolver"));
}

// ── 参数设置 ──

/** @brief 设置收敛阈值 @param tol 残差阈值(默认1e-6) */
void SparseSolver::setTolerance(double tol)
{
    m_tolerance = qMax(1e-15, tol);
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代(默认200) */
void SparseSolver::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/** @brief 设置预条件类型 @param pc 预条件器 */
void SparseSolver::setPreconditioner(Preconditioner pc)
{
    m_pc = pc;
}

/** @brief 设置SSOR松弛因子 @param omega 松弛因子(0,2) */
void SparseSolver::setOmega(double omega)
{
    m_omega = qBound(0.01, omega, 1.99);
}

// ── 矩阵构建 ──

/**
 * @brief 从三元组构建CSR稀疏矩阵
 * @param n 矩阵维度
 * @param triples (row, col, value)三元组列表
 *
 * 将三元组按行排序，合并相同(row,col)的值，构建CSR格式的稀疏矩阵。
 * 同时提取对角线元素供预条件使用。
 */
void SparseSolver::buildMatrix(int n,
    const QVector<QTriple<int, int, double>>& triples)
{
    m_matrix.n = n;
    m_matrix.rowPtr.resize(n + 1, 0);
    m_matrix.colIdx.clear();
    m_matrix.values.clear();

    /* 按行分组统计每行非零元素数 */
    QVector<QVector<QPair<int, double>>> rows(n);
    for (const auto& t : triples) {
        int r = qBound(0, t.first, n - 1);
        int c = qBound(0, t.second, n - 1);
        rows[r].append({c, t.third});
    }

    /* 合并相同列并排序 */
    for (int i = 0; i < n; ++i) {
        QMap<int, double> merged;
        for (const auto& p : rows[i]) {
            merged[p.first] += p.second;
        }
        rows[i].clear();
        for (auto it = merged.constBegin(); it != merged.constEnd(); ++it) {
            if (qAbs(it.value()) > 1e-18) {
                rows[i].append({it.key(), it.value()});
            }
        }
        std::sort(rows[i].begin(), rows[i].end(),
            [](const QPair<int, double>& a, const QPair<int, double>& b) {
                return a.first < b.first;
            });
    }

    /* 构建CSR */
    m_matrix.rowPtr[0] = 0;
    for (int i = 0; i < n; ++i) {
        for (const auto& p : rows[i]) {
            m_matrix.colIdx.append(p.first);
            m_matrix.values.append(p.second);
        }
        m_matrix.rowPtr[i + 1] = m_matrix.values.size();
    }

    extractDiagonal();
}

// ── 核心求解 ──

/**
 * @brief 使用预条件共轭梯度法求解 Ax = b
 * @param b 右端向量
 * @return 求解结果(含解向量、迭代数、残差、收敛标志)
 *
 * 算法流程:
 * 1. 初始化 x0 = 0, r0 = b - Ax0, z0 = M^{-1}r0, p0 = z0
 * 2. 迭代: alpha = rz / pAp, x += alpha*p, r -= alpha*Ap
 * 3. 检查残差, 收敛则停止
 * 4. 预条件: z = M^{-1}r, beta = rz_new / rz_old, p = z + beta*p
 */
SparseSolver::SolveResult SparseSolver::solve(const QVector<double>& b)
{
    SolveResult result;
    int n = m_matrix.n;
    if (n == 0 || b.size() != n) {
        emit solveCompleted(result);
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    /* 初始化 */
    QVector<double> x(n, 0.0);
    QVector<double> r = b;  /* r = b - A*0 = b */
    QVector<double> z = applyPreconditioner(r);
    QVector<double> p = z;
    double rz = dot(r, z);

    double bNorm = norm(b);
    if (bNorm < 1e-18) {
        result.solution = x;
        result.converged = true;
        result.residualNorm = 0.0;
        result.iterations = 0;
        result.elapsedMs = timer.elapsed();
        emit solveCompleted(result);
        return result;
    }

    /* CG迭代 */
    int iter = 0;
    for (iter = 1; iter <= m_maxIter; ++iter) {
        QVector<double> ap = multiply(p);
        double pap = dot(p, ap);

        if (qAbs(pap) < 1e-18) break;

        double alpha = rz / pap;

        /* x += alpha * p */
        for (int i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * ap[i];
        }

        double rNorm = norm(r);
        result.residualNorm = rNorm;

        if (rNorm / bNorm < m_tolerance) {
            result.converged = true;
            break;
        }

        /* 预条件 */
        z = applyPreconditioner(r);
        double rzNew = dot(r, z);
        double beta = rzNew / rz;

        /* p = z + beta * p */
        for (int i = 0; i < n; ++i) {
            p[i] = z[i] + beta * p[i];
        }

        rz = rzNew;
    }

    result.solution = x;
    result.iterations = iter;
    result.elapsedMs = timer.elapsed();

    /* 更新统计 */
    ++m_stats.totalSolves;
    m_stats.totalIterations += iter;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);
    if (result.converged) {
        ++m_stats.totalConverged;
    } else {
        ++m_stats.totalDiverged;
    }

    emit solveCompleted(result);
    return result;
}

// ── 矩阵向量乘法 ──

/**
 * @brief 稀疏矩阵-向量乘法 y = Ax
 * @param x 输入向量
 * @return 结果向量
 */
QVector<double> SparseSolver::multiply(const QVector<double>& x) const
{
    int n = m_matrix.n;
    QVector<double> y(n, 0.0);
    if (x.size() != n) return y;

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = m_matrix.rowPtr[i]; j < m_matrix.rowPtr[i + 1]; ++j) {
            sum += m_matrix.values[j] * x[m_matrix.colIdx[j]];
        }
        y[i] = sum;
    }
    return y;
}

// ── 预条件器 ──

/**
 * @brief 应用预条件器 M^{-1}r -> z
 * @param r 残差向量
 * @return 预条件后的向量
 */
QVector<double> SparseSolver::applyPreconditioner(
    const QVector<double>& r) const
{
    switch (m_pc) {
    case Preconditioner::Jacobi:
        return jacobiPrecond(r);
    case Preconditioner::SSOR:
        return ssorBackward(ssorForward(r));
    case Preconditioner::None:
    default:
        return r;
    }
}

/** @brief Jacobi预条件: z_i = r_i / d_i */
QVector<double> SparseSolver::jacobiPrecond(
    const QVector<double>& r) const
{
    int n = m_matrix.n;
    QVector<double> z(n, 0.0);
    for (int i = 0; i < n; ++i) {
        z[i] = (qAbs(m_diag[i]) > 1e-18) ? r[i] / m_diag[i] : r[i];
    }
    return z;
}

/** @brief SSOR前向回代: (D/omega + L) z' = r */
QVector<double> SparseSolver::ssorForward(
    const QVector<double>& r) const
{
    int n = m_matrix.n;
    QVector<double> z(n, 0.0);
    double w = m_omega;

    for (int i = 0; i < n; ++i) {
        double sum = r[i];
        for (int j = m_matrix.rowPtr[i]; j < m_matrix.rowPtr[i + 1]; ++j) {
            int col = m_matrix.colIdx[j];
            if (col < i) {
                sum -= m_matrix.values[j] * z[col];
            }
        }
        double diag = (qAbs(m_diag[i]) > 1e-18) ? m_diag[i] : 1.0;
        z[i] = sum * w / diag;
    }
    return z;
}

/** @brief SSOR后向回代: (D/omega + U) z = D/omega * z' */
QVector<double> SparseSolver::ssorBackward(
    const QVector<double>& r) const
{
    int n = m_matrix.n;
    QVector<double> z = r;
    double w = m_omega;

    for (int i = n - 1; i >= 0; --i) {
        double sum = z[i];
        for (int j = m_matrix.rowPtr[i]; j < m_matrix.rowPtr[i + 1]; ++j) {
            int col = m_matrix.colIdx[j];
            if (col > i) {
                sum -= m_matrix.values[j] * z[col];
            }
        }
        double diag = (qAbs(m_diag[i]) > 1e-18) ? m_diag[i] : 1.0;
        z[i] = sum * w / diag;
    }
    return z;
}

// ── 辅助方法 ──

/** @brief 提取对角线元素 */
void SparseSolver::extractDiagonal()
{
    int n = m_matrix.n;
    m_diag.resize(n);
    for (int i = 0; i < n; ++i) {
        m_diag[i] = 0.0;
        for (int j = m_matrix.rowPtr[i]; j < m_matrix.rowPtr[i + 1]; ++j) {
            if (m_matrix.colIdx[j] == i) {
                m_diag[i] = m_matrix.values[j];
                break;
            }
        }
    }
}

/** @brief 内积 */
double SparseSolver::dot(const QVector<double>& a,
                          const QVector<double>& b)
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

/** @brief 向量范数 */
double SparseSolver::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/** @brief 重置统计 */
void SparseSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
