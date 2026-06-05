/**
 * @file SparseGMRES2.cpp
 * @brief 稀疏GMRES2实现 — 重启GMRES+预条件ILU(0)
 *
 * 实现广义最小残差法(GMRES)的重启版本，支持ILU(0)预条件。
 * 适用于大型稀疏非对称线性方程组 Ax=b 的高效求解。
 */

#include "utils/matrix48/SparseGMRES2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
SparseGMRES2::SparseGMRES2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置稀疏矩阵(CSR格式)
 * @param n 矩阵维度
 * @param rowPtr 行偏移数组，大小为 n+1
 * @param colIdx 列索引数组
 * @param values 非零元素值数组
 */
void SparseGMRES2::setMatrix(int n, const QVector<int>& rowPtr,
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
 * @brief 设置重启维度
 * @param m Krylov子空间维度，默认30
 */
void SparseGMRES2::setRestart(int m)
{
    m_restart = qMax(1, m);
}

/**
 * @brief 设置收敛容差
 * @param tol 相对残差容差，默认1e-8
 */
void SparseGMRES2::setTolerance(double tol)
{
    m_tol = qMax(1e-15, tol);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大总迭代次数，默认1000
 */
void SparseGMRES2::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 构建ILU(0)预条件子
 * @return true成功构建，false矩阵未设置
 *
 * ILU(0)在不填充(incomplete)条件下进行LU分解，
 * 保持与原矩阵相同的稀疏模式。
 */
bool SparseGMRES2::buildPreconditioner()
{
    if (m_n == 0 || m_rowPtr.isEmpty()) return false;

    buildILU0();
    m_precondBuilt = true;
    return true;
}

/**
 * @brief 求解线性方程组 Ax = b
 * @param rhs 右端向量b
 * @return 解向量x
 *
 * 重启GMRES(m)算法:
 * 1. 计算初始残差 r0 = b - A*x0
 * 2. 构建Krylov子空间的正交基(Arnoldi过程)
 * 3. 求解最小二乘问题
 * 4. 每m步重启
 */
QVector<double> SparseGMRES2::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || rhs.size() != m_n) {
        return QVector<double>(m_n, 0.0);
    }

    QVector<double> x(m_n, 0.0);    /* 初始猜测为零向量 */
    QVector<double> b = rhs;
    int totalIter = 0;
    int totalRestarts = 0;

    /* ---- GMRES(m) 外层循环 ---- */
    for (int outer = 0; outer < m_maxIter / m_restart + 1; ++outer) {
        /* 计算残差 r = M^{-1}(b - Ax) */
        QVector<double> Ax = sparseMultiply(x);
        QVector<double> r(m_n);
        for (int i = 0; i < m_n; ++i) {
            r[i] = b[i] - Ax[i];
        }

        if (m_precondBuilt) {
            r = iluSolve(r);
        }

        double beta = 0.0;
        for (int i = 0; i < m_n; ++i) {
            beta += r[i] * r[i];
        }
        beta = qSqrt(beta);

        if (beta < 1e-15) break;

        double bNorm = 0.0;
        for (int i = 0; i < m_n; ++i) bNorm += b[i] * b[i];
        bNorm = qSqrt(bNorm);
        if (bNorm < 1e-15) bNorm = 1.0;

        /* Arnoldi向量的正交基 */
        QVector<QVector<double>> V(m_restart + 1, QVector<double>(m_n, 0.0));
        for (int i = 0; i < m_n; ++i) {
            V[0][i] = r[i] / beta;
        }

        /* Hessenberg矩阵 */
        QVector<QVector<double>> H(m_restart + 1, QVector<double>(m_restart, 0.0));

        /* Givens旋转参数 */
        QVector<double> cs(m_restart, 0.0);
        QVector<double> sn(m_restart, 0.0);
        QVector<double> g(m_restart + 1, 0.0);
        g[0] = beta;

        int j = 0;
        for (j = 0; j < m_restart && totalIter < m_maxIter; ++j) {
            /* w = M^{-1} * A * V[j] */
            QVector<double> w = sparseMultiply(V[j]);
            if (m_precondBuilt) {
                w = iluSolve(w);
            }

            /* Modified Gram-Schmidt正交化 */
            for (int i = 0; i <= j; ++i) {
                double dot = 0.0;
                for (int k = 0; k < m_n; ++k) {
                    dot += w[k] * V[i][k];
                }
                H[i][j] = dot;
                for (int k = 0; k < m_n; ++k) {
                    w[k] -= H[i][j] * V[i][k];
                }
            }

            double wNorm = 0.0;
            for (int k = 0; k < m_n; ++k) wNorm += w[k] * w[k];
            H[j + 1][j] = qSqrt(wNorm);

            if (H[j + 1][j] > 1e-15) {
                for (int k = 0; k < m_n; ++k) {
                    V[j + 1][k] = w[k] / H[j + 1][j];
                }
            }

            /* 应用之前的Givens旋转 */
            for (int i = 0; i < j; ++i) {
                double temp = cs[i] * H[i][j] + sn[i] * H[i + 1][j];
                H[i + 1][j] = -sn[i] * H[i][j] + cs[i] * H[i + 1][j];
                H[i][j] = temp;
            }

            /* 计算新的Givens旋转 */
            double rVal = H[j][j];
            double iVal = H[j + 1][j];
            double rotNorm = qSqrt(rVal * rVal + iVal * iVal);
            if (rotNorm > 1e-15) {
                cs[j] = rVal / rotNorm;
                sn[j] = iVal / rotNorm;
            } else {
                cs[j] = 1.0;
                sn[j] = 0.0;
            }

            H[j][j] = cs[j] * H[j][j] + sn[j] * H[j + 1][j];
            H[j + 1][j] = 0.0;

            g[j + 1] = -sn[j] * g[j];
            g[j] = cs[j] * g[j];

            totalIter++;

            /* 收敛检查 */
            double relRes = qFabs(g[j + 1]) / bNorm;
            if (relRes < m_tol) break;
        }

        /* 回代求解最小二乘问题 */
        QVector<double> y(j, 0.0);
        for (int i = j - 1; i >= 0; --i) {
            y[i] = g[i];
            for (int k = i + 1; k < j; ++k) {
                y[i] -= H[i][k] * y[k];
            }
            y[i] /= (qFabs(H[i][i]) > 1e-15) ? H[i][i] : 1.0;
        }

        /* 更新解向量 */
        for (int i = 0; i < j; ++i) {
            for (int k = 0; k < m_n; ++k) {
                x[k] += y[i] * V[i][k];
            }
        }

        /* 计算残差范数 */
        m_residualNorm = qFabs(g[j]);
        totalRestarts++;

        if (m_residualNorm / bNorm < m_tol) break;
    }

    m_iterationsUsed = totalIter;

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolves++;
    m_stats.totalIterations += totalIter;
    m_stats.totalRestarts += totalRestarts;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(totalIter, m_residualNorm);
    return x;
}

/**
 * @brief 稀疏矩阵向量乘法 y = Ax
 * @param x 输入向量
 * @return 结果向量y
 */
QVector<double> SparseGMRES2::sparseMultiply(const QVector<double>& x) const
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
 * @brief ILU(0)预条件求解 Mz = r
 * @param r 右端向量
 * @return 预条件后的向量z
 */
QVector<double> SparseGMRES2::iluSolve(const QVector<double>& r) const
{
    const int n = r.size();
    QVector<double> z(n, 0.0);

    /* 前向替换 Lz = r */
    for (int i = 0; i < n; ++i) {
        double sum = r[i];
        for (int idx = m_iluRowPtr[i]; idx < m_iluRowPtr[i + 1]; ++idx) {
            int col = m_iluColIdx[idx];
            if (col < i) {
                sum -= m_iluValues[idx] * z[col];
            }
        }
        z[i] = sum;
    }

    /* 后向替换 Uz' = z */
    for (int i = n - 1; i >= 0; --i) {
        double diag = 1.0;
        double sum = z[i];
        for (int idx = m_iluRowPtr[i]; idx < m_iluRowPtr[i + 1]; ++idx) {
            int col = m_iluColIdx[idx];
            if (col > i) {
                sum -= m_iluValues[idx] * z[col];
            } else if (col == i) {
                diag = m_iluValues[idx];
            }
        }
        z[i] = (qFabs(diag) > 1e-15) ? sum / diag : sum;
    }

    return z;
}

/**
 * @brief 构建ILU(0)分解
 *
 * ILU(0)在原矩阵稀疏模式上进行不完全LU分解。
 * 分解后的L和U共享原矩阵的稀疏结构。
 */
void SparseGMRES2::buildILU0()
{
    m_iluRowPtr = m_rowPtr;
    m_iluColIdx = m_colIdx;
    m_iluValues = m_values;

    for (int i = 0; i < m_n; ++i) {
        for (int idx = m_iluRowPtr[i]; idx < m_iluRowPtr[i + 1]; ++idx) {
            int j = m_iluColIdx[idx];
            if (j >= i) continue;

            /* 找对角元素 */
            double diag = 1.0;
            for (int d = m_iluRowPtr[i]; d < m_iluRowPtr[i + 1]; ++d) {
                if (m_iluColIdx[d] == i) { diag = m_iluValues[d]; break; }
            }
            if (qFabs(diag) < 1e-15) continue;

            m_iluValues[idx] /= diag;

            /* 更新行i中j列右侧的元素 */
            for (int idx2 = idx + 1; idx2 < m_iluRowPtr[i + 1]; ++idx2) {
                int k = m_iluColIdx[idx2];
                /* 在行j中找列k */
                for (int idx3 = m_iluRowPtr[j]; idx3 < m_iluRowPtr[j + 1]; ++idx3) {
                    if (m_iluColIdx[idx3] == k) {
                        m_iluValues[idx2] -= m_iluValues[idx] * m_iluValues[idx3];
                        break;
                    }
                }
            }
        }
    }
}

/**
 * @brief 重置所有统计信息
 */
void SparseGMRES2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
