/**
 * @file SparseGMRES.cpp
 * @brief 稀疏GMRES求解器实现 — Arnoldi过程/重启GMRES/预条件
 */

#include "utils/matrix33/SparseGMRES.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SparseGMRES::SparseGMRES(QObject* parent)
    : QObject(parent)
{
}

/** @brief 从COO格式构建稀疏矩阵(CSR) @param rows 行索引 @param cols 列索引 @param vals 值 @param n 维度 */
void SparseGMRES::buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                                 const QVector<double>& vals, int n)
{
    m_n = qMax(1, n);

    /* 构建CSR格式 */
    QVector<int> rowCounts(m_n + 1, 0);

    /* 统计每行非零元素个数 */
    for (int idx = 0; idx < rows.size(); ++idx) {
        if (rows[idx] >= 0 && rows[idx] < m_n) {
            ++rowCounts[rows[idx] + 1];
        }
    }

    /* 累加得到行指针 */
    for (int i = 0; i < m_n; ++i) {
        rowCounts[i + 1] += rowCounts[i];
    }
    m_rowPtr = rowCounts;

    /* 填充列索引和值 */
    int nnz = rows.size();
    m_colIdx.resize(nnz);
    m_values.resize(nnz);

    QVector<int> pos = m_rowPtr;
    for (int idx = 0; idx < rows.size(); ++idx) {
        if (rows[idx] >= 0 && rows[idx] < m_n) {
            int p = pos[rows[idx]]++;
            m_colIdx[p] = cols[idx];
            m_values[p] = vals[idx];
        }
    }

    /* 对每行按列索引排序 */
    for (int i = 0; i < m_n; ++i) {
        int start = m_rowPtr[i];
        int end = m_rowPtr[i + 1];
        for (int j = start; j < end - 1; ++j) {
            for (int k = j + 1; k < end; ++k) {
                if (m_colIdx[j] > m_colIdx[k]) {
                    std::swap(m_colIdx[j], m_colIdx[k]);
                    std::swap(m_values[j], m_values[k]);
                }
            }
        }
    }
}

/** @brief 设置重启参数 @param m Krylov子空间维度 */
void SparseGMRES::setRestart(int m)
{
    m_restart = qMax(5, m);
}

/** @brief 设置收敛容差 @param tol 容差 */
void SparseGMRES::setTolerance(double tol)
{
    m_tol = qMax(1e-15, tol);
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void SparseGMRES::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/** @brief 求解线性系统Ax=rhs @param rhs 右端向量 @return 解向量 */
QVector<double> SparseGMRES::solve(const QVector<double>& rhs)
{
    /* 使用单位预条件 */
    QVector<double> prec(m_n, 1.0);
    return solveWithPrec(rhs, prec);
}

/** @brief 带对角预条件的GMRES求解 @param rhs 右端向量 @param precDiag 预条件对角元 @return 解向量 */
QVector<double> SparseGMRES::solveWithPrec(const QVector<double>& rhs,
                                             const QVector<double>& precDiag)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_n, 0.0);
    QVector<double> r = rhs; /* 初始残差 r = b - A*x (x=0) */

    /* 对角预条件: r = M^{-1} * r */
    for (int i = 0; i < m_n; ++i) {
        double d = (i < precDiag.size() && qFabs(precDiag[i]) > 1e-15)
            ? precDiag[i] : 1.0;
        r[i] /= d;
    }

    double rhsNorm = 0.0;
    for (int i = 0; i < m_n; ++i) rhsNorm += rhs[i] * rhs[i];
    rhsNorm = qSqrt(rhsNorm);

    if (rhsNorm < 1e-15) {
        m_stats.totalSolves++;
        m_lastIter = 0;
        return x;
    }

    double beta = 0.0;
    for (int i = 0; i < m_n; ++i) beta += r[i] * r[i];
    beta = qSqrt(beta);

    m_lastIter = 0;
    bool converged = false;

    for (int outer = 0; outer < m_maxIter; outer += m_restart) {
        /* 初始化Krylov子空间 */
        int m = qMin(m_restart, m_maxIter - outer);
        QVector<QVector<double>> V(m + 1, QVector<double>(m_n, 0.0));
        QVector<double> sn(m, 0.0);
        QVector<double> cs(m, 0.0);
        QVector<double> g(m + 1, 0.0);
        QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));

        /* v1 = r / beta */
        for (int i = 0; i < m_n; ++i) {
            V[0][i] = r[i] / beta;
        }
        g[0] = beta;

        for (int j = 0; j < m; ++j) {
            /* Arnoldi过程 */
            arnoldi(V, H[j], j, precDiag);

            /* 上Hessenberg矩阵的QR分解(Givens旋转) */
            for (int i = 0; i < j; ++i) {
                double temp = cs[i] * H[j][i] + sn[i] * H[j][i + 1];
                H[j][i + 1] = -sn[i] * H[j][i] + cs[i] * H[j][i + 1];
                H[j][i] = temp;
            }

            double hVal = qSqrt(H[j][j] * H[j][j] + H[j][j + 1] * H[j][j + 1]);
            cs[j] = H[j][j] / hVal;
            sn[j] = H[j][j + 1] / hVal;
            H[j][j] = cs[j] * H[j][j] + sn[j] * H[j][j + 1];
            H[j][j + 1] = 0.0;

            g[j + 1] = -sn[j] * g[j];
            g[j] = cs[j] * g[j];

            ++m_lastIter;
            double residual = qFabs(g[j + 1]) / rhsNorm;
            if (residual < m_tol) {
                /* 回代求解 */
                QVector<double> y(j + 1, 0.0);
                for (int ii = j; ii >= 0; --ii) {
                    y[ii] = g[ii];
                    for (int kk = ii + 1; kk <= j; ++kk) {
                        y[ii] -= H[kk][ii] * y[kk];
                    }
                    y[ii] /= (qFabs(H[ii][ii]) > 1e-15) ? H[ii][ii] : 1e-15;
                }
                for (int ii = 0; ii <= j; ++ii) {
                    for (int kk = 0; kk < m_n; ++kk) {
                        x[kk] += y[ii] * V[ii][kk];
                    }
                }
                converged = true;
                break;
            }
        }

        if (converged) break;

        /* 重启: 计算新的残差 */
        int mm = m;
        QVector<double> y(mm, 0.0);
        for (int i = mm - 1; i >= 0; --i) {
            y[i] = g[i];
            for (int k = i + 1; k < mm; ++k) {
                y[i] -= H[k][i] * y[k];
            }
            y[i] /= (qFabs(H[i][i]) > 1e-15) ? H[i][i] : 1e-15;
        }
        for (int i = 0; i < mm; ++i) {
            for (int k = 0; k < m_n; ++k) {
                x[k] += y[i] * V[i][k];
            }
        }

        r = rhs;
        QVector<double> Ax = spmv(x);
        for (int i = 0; i < m_n; ++i) r[i] -= Ax[i];
        for (int i = 0; i < m_n; ++i) {
            double d = (i < precDiag.size() && qFabs(precDiag[i]) > 1e-15)
                ? precDiag[i] : 1.0;
            r[i] /= d;
        }
        beta = 0.0;
        for (int i = 0; i < m_n; ++i) beta += r[i] * r[i];
        beta = qSqrt(beta);
    }

    m_stats.totalSolves++;
    m_stats.totalIterations += m_lastIter;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSolves));

    emit solveComplete(converged, m_lastIter);
    return x;
}

/** @brief 稀疏矩阵向量乘 @param x 向量 @return Ax */
QVector<double> SparseGMRES::spmv(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
            int col = m_colIdx[j];
            if (col >= 0 && col < x.size()) {
                sum += m_values[j] * x[col];
            }
        }
        y[i] = sum;
    }
    return y;
}

/** @brief Arnoldi过程: 计算下一个Krylov向量 @param V 正交基 @param h Hessenberg列 @param j 当前步 @param rhs 仅签名兼容 */
void SparseGMRES::arnoldi(QVector<QVector<double>>& V, QVector<double>& h,
                           int j, const QVector<double>& rhs)
{
    /* w = A * V[j] */
    QVector<double> w = spmv(V[j]);

    /* 预条件 */
    for (int i = 0; i < m_n; ++i) {
        if (i < rhs.size() && qFabs(rhs[i]) > 1e-15) {
            w[i] /= rhs[i];
        }
    }

    /* Modified Gram-Schmidt正交化 */
    for (int i = 0; i <= j; ++i) {
        h[i] = 0.0;
        for (int k = 0; k < m_n; ++k) {
            h[i] += w[k] * V[i][k];
        }
        for (int k = 0; k < m_n; ++k) {
            w[k] -= h[i] * V[i][k];
        }
    }

    /* 归一化 */
    double normW = 0.0;
    for (int i = 0; i < m_n; ++i) normW += w[i] * w[i];
    h[j + 1] = qSqrt(normW);

    if (h[j + 1] > 1e-15) {
        for (int i = 0; i < m_n; ++i) {
            V[j + 1][i] = w[i] / h[j + 1];
        }
    }
}

/** @brief 获取上次求解的迭代次数 @return 迭代次数 */
int SparseGMRES::iterations() const
{
    return m_lastIter;
}

/** @brief 重置统计 */
void SparseGMRES::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
