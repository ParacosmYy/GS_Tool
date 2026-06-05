/**
 * @file KrylovSolver.cpp
 * @brief Krylov子空间方法实现 — GMRES迭代求解大规模稀疏线性系统
 */

#include "utils/matrix16/KrylovSolver.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/* ── 构造/配置 ── */

/** @brief 构造函数 @param parent 父对象 */
KrylovSolver::KrylovSolver(QObject* parent)
    : QObject(parent)
    , m_dimension(0)
    , m_maxIter(100)
    , m_tolerance(1e-8)
    , m_precond(Preconditioner::None)
{
}

/** @brief 设置矩阵维度 @param n 维度 */
void KrylovSolver::setDimension(int n)
{
    m_dimension = qMax(0, n);
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void KrylovSolver::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/** @brief 设置收敛容差 @param tol 容差 */
void KrylovSolver::setTolerance(double tol)
{
    m_tolerance = qMax(1e-15, tol);
}

/** @brief 设置预处理类型 @param pc 预处理类型 */
void KrylovSolver::setPreconditioner(Preconditioner pc)
{
    m_precond = pc;
}

/** @brief 设置稀疏矩阵(COO转CSR) @param entries 三元组列表 */
void KrylovSolver::setSparseMatrix(const QVector<SparseEntry>& entries)
{
    /* 清空旧数据 */
    m_rowPtr.clear();
    m_colIdx.clear();
    m_values.clear();
    m_diagInv.clear();

    if (m_dimension <= 0 || entries.isEmpty()) return;

    m_rowPtr.resize(m_dimension + 1, 0);
    m_diagInv.resize(m_dimension, 1.0);

    /* 统计每行非零数 */
    for (const auto& e : entries) {
        if (e.row >= 0 && e.row < m_dimension) {
            ++m_rowPtr[e.row + 1];
        }
    }
    /* 前缀和生成行指针 */
    for (int i = 0; i < m_dimension; ++i) {
        m_rowPtr[i + 1] += m_rowPtr[i];
    }

    /* 填充列索引和值 */
    QVector<int> pos = m_rowPtr;
    m_colIdx.resize(m_rowPtr[m_dimension]);
    m_values.resize(m_rowPtr[m_dimension]);

    for (const auto& e : entries) {
        if (e.row >= 0 && e.row < m_dimension
            && e.col >= 0 && e.col < m_dimension) {
            int idx = pos[e.row]++;
            m_colIdx[idx] = e.col;
            m_values[idx] = e.value;
            /* 记录对角线逆 */
            if (e.row == e.col && std::abs(e.value) > 1e-15) {
                m_diagInv[e.row] = 1.0 / e.value;
            }
        }
    }
}

/* ── 公共接口 ── */

/** @brief GMRES求解Ax=b @param b 右端向量 @return 求解结果 */
KrylovSolver::SolveResult KrylovSolver::solve(const QVector<double>& b)
{
    SolveResult result;
    if (m_dimension <= 0 || b.size() != m_dimension) return result;

    QElapsedTimer timer;
    timer.start();

    int n = m_dimension;
    int maxK = qMin(m_maxIter, n);

    /* 初始猜测x0=0, 残差r0=b */
    result.x.resize(n, 0.0);
    QVector<double> r = b;

    double bNorm = 0.0;
    for (double v : b) bNorm += v * v;
    bNorm = std::sqrt(bNorm);
    if (bNorm < 1e-15) {
        result.converged = true;
        result.residualNorm = 0.0;
        return result;
    }

    /* 预处理残差 */
    applyPreconditioner(r);
    double beta = 0.0;
    for (double v : r) beta += v * v;
    beta = std::sqrt(beta);

    /* Krylov基向量V */
    QVector<QVector<double>> V(maxK + 1, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[0][i] = r[i] / beta;

    /* 上Hessenberg矩阵H(列为非零部分) */
    QVector<QVector<double>> H(maxK + 1, QVector<double>(maxK, 0.0));

    /* Givens旋转参数 */
    QVector<double> cs(maxK, 0.0);
    QVector<double> sn(maxK, 0.0);
    QVector<double> g(maxK + 1, 0.0);
    g[0] = beta;

    int k = 0;
    for (k = 0; k < maxK; ++k) {
        /* Arnoldi步: V[k+1] = A*V[k]，然后正交化 */
        QVector<double> w = multiply(V[k]);
        applyPreconditioner(w);

        for (int i = 0; i <= k; ++i) {
            double dot = 0.0;
            for (int j = 0; j < n; ++j) dot += w[j] * V[i][j];
            H[i][k] = dot;
            for (int j = 0; j < n; ++j) w[j] -= dot * V[i][j];
        }

        double wNorm = 0.0;
        for (double v : w) wNorm += v * v;
        wNorm = std::sqrt(wNorm);
        H[k + 1][k] = wNorm;

        if (wNorm < 1e-14) {
            /* Krylov子空间不变，提前终止 */
            ++k;
            break;
        }

        for (int j = 0; j < n; ++j) V[k + 1][j] = w[j] / wNorm;

        /* 应用之前的Givens旋转到新列 */
        for (int i = 0; i < k; ++i) {
            double tmp = cs[i] * H[i][k] + sn[i] * H[i + 1][k];
            H[i + 1][k] = -sn[i] * H[i][k] + cs[i] * H[i + 1][k];
            H[i][k] = tmp;
        }

        /* 计算新的Givens旋转 */
        double rr = std::sqrt(H[k][k] * H[k][k] + H[k + 1][k] * H[k + 1][k]);
        if (rr < 1e-15) rr = 1e-15;
        cs[k] = H[k][k] / rr;
        sn[k] = H[k + 1][k] / rr;

        /* 旋转H和g */
        H[k][k] = cs[k] * H[k][k] + sn[k] * H[k + 1][k];
        H[k + 1][k] = 0.0;
        g[k + 1] = -sn[k] * g[k];
        g[k] = cs[k] * g[k];

        double residual = std::abs(g[k + 1]) / bNorm;
        emit iterationProgress(k + 1, residual);

        if (residual < m_tolerance) {
            ++k;
            break;
        }
    }

    /* 回代上三角系统 Hy = g */
    QVector<double> y(k, 0.0);
    for (int i = k - 1; i >= 0; --i) {
        y[i] = g[i];
        for (int j = i + 1; j < k; ++j) {
            y[i] -= H[i][j] * y[j];
        }
        if (std::abs(H[i][i]) > 1e-15) {
            y[i] /= H[i][i];
        }
    }

    /* 重构解 x = V*y */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            result.x[i] += V[j][i] * y[j];
        }
    }

    /* 计算最终残差 */
    QVector<double> ax = multiply(result.x);
    double resNorm = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = b[i] - ax[i];
        resNorm += d * d;
    }
    result.residualNorm = std::sqrt(resNorm);
    result.iterationsUsed = k;
    result.converged = (result.residualNorm / bNorm) < m_tolerance;

    /* 更新统计 */
    ++m_stats.totalSolves;
    m_stats.totalIterations += k;
    if (result.converged) ++m_stats.totalConverged;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    return result;
}

/** @brief 稀疏矩阵向量乘积y=A*x @param x 输入向量 @return 乘积 */
QVector<double> KrylovSolver::multiply(const QVector<double>& x) const
{
    QVector<double> y(m_dimension, 0.0);
    if (x.size() != m_dimension) return y;

    for (int i = 0; i < m_dimension; ++i) {
        double sum = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
            sum += m_values[j] * x[m_colIdx[j]];
        }
        y[i] = sum;
    }
    return y;
}

/* ── 私有 ── */

/** @brief 对角预处理(Jacobi缩放) @param x 输入/输出向量 */
void KrylovSolver::applyPreconditioner(QVector<double>& x) const
{
    if (m_precond == Preconditioner::None) return;
    if (m_precond == Preconditioner::Diagonal) {
        for (int i = 0; i < m_dimension; ++i) {
            x[i] *= m_diagInv[i];
        }
    }
    /* SymmetricGaussSeidel 简化: 两遍对角缩放 */
    if (m_precond == Preconditioner::SymmetricGaussSeidel) {
        for (int i = 0; i < m_dimension; ++i) {
            x[i] *= m_diagInv[i];
        }
        for (int i = m_dimension - 1; i >= 0; --i) {
            x[i] *= m_diagInv[i];
        }
    }
}

/* ── 统计 ── */

/** @brief 重置统计 */
void KrylovSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
