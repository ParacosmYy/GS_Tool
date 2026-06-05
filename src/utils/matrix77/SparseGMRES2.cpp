/**
 * @file SparseGMRES2.cpp
 * @brief 稀疏矩阵GMRES求解器实现
 *
 * 实现广义最小残差法(GMRES)求解稀疏线性方程组Ax=b。
 * 使用Arnoldi迭代构建Krylov子空间，配合restarted策略
 * 控制内存使用。支持COO稀疏存储格式。
 */

#include "utils/matrix77/SparseGMRES2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认: 最大迭代100次，容差1e-8，重启步数30
 */
SparseGMRES2::SparseGMRES2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param n 矩阵行/列数
 */
void SparseGMRES2::setDimension(int n)
{
    m_n = qMax(0, n);
    m_rowPtr.clear();
    m_colIdx.clear();
    m_values.clear();
}

/**
 * @brief 添加稀疏矩阵元素(COO格式)
 * @param row 行索引
 * @param col 列索引
 * @param val 非零值
 *
 * 以COO格式暂存元素，solve时直接使用进行SpMV。
 */
void SparseGMRES2::addEntry(int row, int col, double val)
{
    if (row < 0 || row >= m_n || col < 0 || col >= m_n) return;
    if (qFuzzyIsNull(val)) return;
    m_rowPtr.append(row);
    m_colIdx.append(col);
    m_values.append(val);
}

/**
 * @brief 设置最大迭代次数
 * @param iter 最大迭代次数
 */
void SparseGMRES2::setMaxIterations(int iter)
{
    m_maxIter = qMax(1, iter);
}

/**
 * @brief 设置收敛容差
 * @param tol 相对残差容差
 */
void SparseGMRES2::setTolerance(double tol)
{
    m_tol = qBound(1e-15, tol, 1.0);
}

/**
 * @brief 设置重启步数
 * @param m GMRES(m)的重启步数
 */
void SparseGMRES2::setRestart(int m)
{
    m_restart = qMax(1, m);
}

/**
 * @brief 稀疏矩阵向量乘法
 * @param x 输入向量
 * @return 结果向量y = A*x
 *
 * 使用COO格式的稀疏矩阵进行SpMV运算。
 */
QVector<double> SparseGMRES2::spMV(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_rowPtr.size(); ++i) {
        int r = m_rowPtr[i];
        int c = m_colIdx[i];
        if (r < m_n && c < x.size()) {
            y[r] += m_values[i] * x[c];
        }
    }
    return y;
}

/**
 * @brief 求解稀疏线性方程组Ax=b
 * @param b 右端项向量
 * @return 解向量x
 *
 * GMRES(m) restarted算法流程:
 * 1. 计算初始残差 r0 = b - A*x0
 * 2. Arnoldi过程构建Krylov子空间的正交基V
 * 3. 构建上Hessenberg矩阵H
 * 4. Givens旋转将H三角化
 * 5. 回代求解最小二乘问题
 * 6. 更新解向量 x = x0 + V*y
 * 7. 检查收敛，未收敛则重启
 */
QVector<double> SparseGMRES2::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_n, 0.0);
    if (m_n == 0 || m_rowPtr.isEmpty()) return x;

    int m = qMin(m_restart, m_n);
    m_iterUsed = 0;
    m_residual = 1e18;

    /* 计算初始残差 r0 = b - A*x0 */
    QVector<double> r = b;
    QVector<double> ax = spMV(x);
    for (int i = 0; i < m_n; ++i) r[i] -= ax[i];

    double beta = 0.0;
    for (int i = 0; i < m_n; ++i) beta += r[i] * r[i];
    beta = qSqrt(beta);

    if (beta < 1e-300) {
        m_residual = 0.0;

        qint64 elapsed = timer.elapsed();
        m_stats.totalSolves++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

        emit solveCompleted(0, 0.0);
        return x;
    }

    double bnorm = 0.0;
    for (int i = 0; i < m_n; ++i) bnorm += b[i] * b[i];
    bnorm = qSqrt(qMax(bnorm, 1e-300));

    /* GMRES(m) 外循环(restarted) */
    for (int outer = 0; outer < m_maxIter; ++outer) {
        /* 初始化Arnoldi向量 */
        QVector<QVector<double>> V(m + 1, QVector<double>(m_n, 0.0));
        for (int i = 0; i < m_n; ++i) V[0][i] = r[i] / beta;

        /* 上Hessenberg矩阵和Givens参数 */
        QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));
        QVector<double> cs(m, 0.0), sn(m, 0.0);
        QVector<double> g(m + 1, 0.0);
        g[0] = beta;

        int j;
        for (j = 0; j < m && m_iterUsed < m_maxIter; ++j) {
            /* Arnoldi步骤: w = A * V[j] */
            QVector<double> w = spMV(V[j]);

            /* Modified Gram-Schmidt正交化 */
            for (int i = 0; i <= j; ++i) {
                double dot = 0.0;
                for (int k = 0; k < m_n; ++k) dot += w[k] * V[i][k];
                H[i][j] = dot;
                for (int k = 0; k < m_n; ++k) w[k] -= dot * V[i][k];
            }

            /* 归一化得到V[j+1] */
            double nrm = 0.0;
            for (int k = 0; k < m_n; ++k) nrm += w[k] * w[k];
            H[j + 1][j] = qSqrt(nrm);
            if (H[j + 1][j] > 1e-300) {
                for (int k = 0; k < m_n; ++k) V[j + 1][k] = w[k] / H[j + 1][j];
            }

            /* 应用之前的Givens旋转 */
            for (int i = 0; i < j; ++i) {
                double temp = cs[i] * H[i][j] + sn[i] * H[i + 1][j];
                H[i + 1][j] = -sn[i] * H[i][j] + cs[i] * H[i + 1][j];
                H[i][j] = temp;
            }

            /* 新的Givens旋转 */
            double rr = qSqrt(H[j][j] * H[j][j] + H[j + 1][j] * H[j + 1][j]);
            cs[j] = H[j][j] / qMax(rr, 1e-300);
            sn[j] = H[j + 1][j] / qMax(rr, 1e-300);
            H[j][j] = rr;
            H[j + 1][j] = 0.0;

            /* 更新g向量 */
            g[j + 1] = -sn[j] * g[j];
            g[j] = cs[j] * g[j];

            m_iterUsed++;
            m_residual = qAbs(g[j + 1]) / bnorm;
            if (m_residual < m_tol) break;
        }

        /* 回代求解最小二乘问题 Hy = g */
        QVector<double> y(j, 0.0);
        for (int i = j - 1; i >= 0; --i) {
            y[i] = g[i];
            for (int k = i + 1; k < j; ++k) y[i] -= H[i][k] * y[k];
            y[i] /= (qAbs(H[i][i]) > 1e-300) ? H[i][i] : 1.0;
        }

        /* 更新解向量: x += V * y */
        for (int i = 0; i < j; ++i) {
            for (int k = 0; k < m_n; ++k) {
                x[k] += y[i] * V[i][k];
            }
        }

        if (m_residual < m_tol) break;

        /* 计算新残差用于重启 */
        r = b;
        ax = spMV(x);
        for (int i = 0; i < m_n; ++i) r[i] -= ax[i];
        beta = 0.0;
        for (int i = 0; i < m_n; ++i) beta += r[i] * r[i];
        beta = qSqrt(beta);
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalIterations += m_iterUsed;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_iterUsed, m_residual);
    return x;
}

/**
 * @brief 重置统计信息
 */
void SparseGMRES2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
