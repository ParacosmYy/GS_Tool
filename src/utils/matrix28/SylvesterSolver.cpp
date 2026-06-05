/**
 * @file SylvesterSolver.cpp
 * @brief Sylvester矩阵方程求解器实现
 *
 * 求解连续Sylvester方程 AX + XB = C,
 * 连续Lyapunov方程 AX + XA^T = C,
 * 离散Lyapunov方程 AXA^T - X = C。
 * 核心方法: Schur分解将系数矩阵三角化, 再列式回代求解三角系统。
 */

#include "utils/matrix28/SylvesterSolver.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <vector>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
SylvesterSolver::SylvesterSolver(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 求解连续Sylvester方程 AX + XB = C
 *
 * 算法流程(Bartels-Stewart):
 * 1. 对A做实Schur分解: A = QUQ^T (U为上拟三角)
 * 2. 对B做实Schur分解: B = VSV^T (S为上拟三角)
 * 3. 变换右端: F = Q^T * C * V
 * 4. 求解三角Sylvester方程: UY + YS = F (列式前代/回代)
 * 5. 反变换: X = Q * Y * V^T
 *
 * @param A m×m矩阵(行优先)
 * @param B n×n矩阵(行优先)
 * @param C m×n矩阵(行优先)
 * @param m A的阶数
 * @param n B的阶数
 * @return 解矩阵X(m×n, 行优先)
 */
QVector<double> SylvesterSolver::solveSylvester(const QVector<double>& A,
                                                 const QVector<double>& B,
                                                 const QVector<double>& C,
                                                 int m, int n)
{
    QElapsedTimer timer;
    timer.start();

    if (m <= 0 || n <= 0 || A.size() < m * m || B.size() < n * n || C.size() < m * n) {
        return {};
    }

    /* Schur分解A → (U, Q) */
    QVector<double> U = A;
    QVector<double> Q(m * m, 0.0);
    for (int i = 0; i < m; ++i) Q[i * m + i] = 1.0;
    schurDecompose(U, m, Q);

    /* Schur分解B → (S, V) */
    QVector<double> S = B;
    QVector<double> V(n * n, 0.0);
    for (int i = 0; i < n; ++i) V[i * n + i] = 1.0;
    schurDecompose(S, n, V);

    /* F = Q^T * C * V */
    /* 先算 Q^T * C → tmp(m×n) */
    QVector<double> tmp(m * n, 0.0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < m; ++k) {
                sum += Q[k * m + i] * C[k * n + j];
            }
            tmp[i * n + j] = sum;
        }
    }
    /* 再算 tmp * V → F(m×n) */
    QVector<double> F(m * n, 0.0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += tmp[i * n + k] * V[k * n + j];
            }
            F[i * n + j] = sum;
        }
    }

    /* 求解三角系统 UY + YS = F */
    QVector<double> Y = triangularSolve(U, S, F, m, n);

    /* X = Q * Y * V^T */
    /* 先算 Q * Y → tmp2(m×n) */
    QVector<double> tmp2(m * n, 0.0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < m; ++k) {
                sum += Q[i * m + k] * Y[k * n + j];
            }
            tmp2[i * n + j] = sum;
        }
    }
    /* 再算 tmp2 * V^T → X(m×n) */
    QVector<double> X(m * n, 0.0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += tmp2[i * n + k] * V[j * n + k];
            }
            X[i * n + j] = sum;
        }
    }

    /* 更新统计 */
    m_stats.totalSolves++;
    m_stats.totalSchurDecompositions += 2;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m, n);
    return X;
}

/**
 * @brief 求解连续Lyapunov方程 AX + XA^T = C
 *
 * Lyapunov方程是Sylvester方程在B=A^T时的特例。
 * 简化流程: Schur分解A, 变换C, 求解三角系统。
 *
 * @param A n×n矩阵(行优先)
 * @param C n×n矩阵(行优先)
 * @param n 矩阵阶数
 * @return 解矩阵X(n×n, 行优先)
 */
QVector<double> SylvesterSolver::solveLyapunov(const QVector<double>& A,
                                                const QVector<double>& C, int n)
{
    /* 构造B = A^T */
    QVector<double> AT(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            AT[i * n + j] = A[j * n + i];
        }
    }
    return solveSylvester(A, AT, C, n, n);
}

/**
 * @brief 求解离散Lyapunov方程 AXA^T - X = C
 *
 * 通过向量化变换为标准线性系统:
 * vec(AXA^T) = (A⊗A)vec(X), 因此 ((A⊗A) - I)vec(X) = vec(C)。
 * 对小规模矩阵直接用Gauss消元求解。
 *
 * @param A n×n矩阵
 * @param C n×n矩阵
 * @param n 阶数
 * @return 解矩阵X(n×n, 行优先)
 */
QVector<double> SylvesterSolver::solveDiscreteLyapunov(const QVector<double>& A,
                                                        const QVector<double>& C, int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || A.size() < n * n || C.size() < n * n) {
        return {};
    }

    int nn = n * n;
    /* 构造 M = A⊗A - I */
    std::vector<double> M(nn * nn, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int p = 0; p < n; ++p) {
                for (int q = 0; q < n; ++q) {
                    int row = i * n + p;
                    int col = j * n + q;
                    M[row * nn + col] = A[i * n + j] * A[p * n + q];
                }
            }
        }
    }
    for (int i = 0; i < nn; ++i) {
        M[i * nn + i] -= 1.0;
    }

    /* 右端: vec(C) */
    std::vector<double> rhs(nn);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            rhs[i * n + j] = C[i * n + j];
        }
    }

    /* Gauss消元求解 Mx = rhs */
    for (int col = 0; col < nn; ++col) {
        int pivot = col;
        for (int row = col + 1; row < nn; ++row) {
            if (qFabs(M[row * nn + col]) > qFabs(M[pivot * nn + col])) {
                pivot = row;
            }
        }
        if (pivot != col) {
            for (int k = 0; k < nn; ++k) std::swap(M[col * nn + k], M[pivot * nn + k]);
            std::swap(rhs[col], rhs[pivot]);
        }
        if (qFabs(M[col * nn + col]) < 1e-14) continue;
        for (int row = col + 1; row < nn; ++row) {
            double factor = M[row * nn + col] / M[col * nn + col];
            for (int k = col; k < nn; ++k) {
                M[row * nn + k] -= factor * M[col * nn + k];
            }
            rhs[row] -= factor * rhs[col];
        }
    }
    /* 回代 */
    std::vector<double> x(nn, 0.0);
    for (int i = nn - 1; i >= 0; --i) {
        double sum = rhs[i];
        for (int k = i + 1; k < nn; ++k) {
            sum -= M[i * nn + k] * x[k];
        }
        x[i] = (qFabs(M[i * nn + i]) > 1e-14) ? sum / M[i * nn + i] : 0.0;
    }

    QVector<double> X(n * n);
    for (int i = 0; i < n * n; ++i) {
        X[i] = x[i];
    }

    m_stats.totalSolves++;
    m_stats.totalSchurDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    return X;
}

/**
 * @brief 检查Sylvester方程可解性
 *
 * 方程有唯一解的充要条件: A和B的特征值无交集(Schur谱条件)。
 * 通过Schur分解得到A和B的准对角元素, 检查是否有重叠。
 *
 * @param A m×m矩阵
 * @param B n×n矩阵
 * @param m A的阶数
 * @param n B的阶数
 * @return true=方程有唯一解
 */
bool SylvesterSolver::isSolvable(const QVector<double>& A, const QVector<double>& B,
                                  int m, int n) const
{
    if (m <= 0 || n <= 0) return false;

    /* 提取A的对角元素近似特征值 */
    QVector<double> eigA(m), eigB(n);
    for (int i = 0; i < m; ++i) {
        eigA[i] = A[i * m + i];
    }
    for (int i = 0; i < n; ++i) {
        eigB[i] = B[i * n + i];
    }

    /* 检查是否有特征值过于接近(阈值1e-8) */
    const double tol = 1e-8;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            if (qFabs(eigA[i] - eigB[j]) < tol) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief 重置统计计数器
 * 将求解次数、Schur分解次数和平均时间归零
 */
void SylvesterSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 实Schur分解 — QR迭代法
 *
 * 通过多次QR分解迭代, 将矩阵约化为上拟三角形式。
 * 每次迭代: mat = Q*R, 然后 mat = R*Q (相似变换)。
 * Q累积存储正交变换矩阵。
 *
 * @param mat 输入矩阵(n×n), 输出上拟三角矩阵
 * @param n 矩阵阶数
 * @param Q 输出正交变换矩阵(初始化为单位阵)
 */
void SylvesterSolver::schurDecompose(QVector<double>& mat, int n,
                                      QVector<double>& Q) const
{
    const int maxIter = 100 * n;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* Wilkinson位移: 取右下角2×2块的特征值 */
        double shift = 0.0;
        if (n >= 2) {
            double a = mat[(n - 2) * n + (n - 2)];
            double b = mat[(n - 2) * n + (n - 1)];
            double c = mat[(n - 1) * n + (n - 2)];
            double d = mat[(n - 1) * n + (n - 1)];
            double tr = a + d;
            double det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            if (disc < 0) disc = 0.0;
            shift = (tr + qSqrt(disc)) * 0.5;
        }

        /* 减去位移 */
        for (int i = 0; i < n; ++i) {
            mat[i * n + i] -= shift;
        }

        /* QR分解 (Givens旋转) */
        for (int col = 0; col < n - 1; ++col) {
            for (int row = n - 1; row > col; --row) {
                double a = mat[(row - 1) * n + col];
                double b = mat[row * n + col];
                double r = qSqrt(a * a + b * b);
                if (r < 1e-15) continue;
                double c = a / r;
                double s = -b / r;

                /* 左乘Givens到mat */
                for (int k = 0; k < n; ++k) {
                    double t1 = mat[(row - 1) * n + k];
                    double t2 = mat[row * n + k];
                    mat[(row - 1) * n + k] = c * t1 - s * t2;
                    mat[row * n + k] = s * t1 + c * t2;
                }
                /* 左乘Givens到Q */
                for (int k = 0; k < n; ++k) {
                    double t1 = Q[k * n + (row - 1)];
                    double t2 = Q[k * n + row];
                    Q[k * n + (row - 1)] = c * t1 - s * t2;
                    Q[k * n + row] = s * t1 + c * t2;
                }
            }
        }

        /* 恢复位移: mat = RQ + shift*I */
        for (int i = 0; i < n; ++i) {
            mat[i * n + i] += shift;
        }

        /* 检查下三角是否足够小 */
        double offNorm = 0.0;
        for (int i = 1; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                offNorm += mat[i * n + j] * mat[i * n + j];
            }
        }
        if (offNorm < 1e-20) break;
    }
}

/**
 * @brief 三角Sylvester方程求解 — 列式前代
 *
 * 求解 T1*Y + Y*T2 = F, 其中T1(m×m)和T2(n×n)为上三角矩阵。
 * 按列递推求解:
 * - 第j列: (T1 + T2[j][j]*I) * Y[:,j] = F[:,j] - sum_{k>j}(T2[j][k]*Y[:,k])
 * - 每列是一个三角线性系统, 用前代/回代求解
 *
 * @param T1 上三角矩阵(m×m)
 * @param T2 上三角矩阵(n×n)
 * @param F 右端矩阵(m×n)
 * @param m T1阶数
 * @param n T2阶数
 * @return 解矩阵Y(m×n, 行优先)
 */
QVector<double> SylvesterSolver::triangularSolve(const QVector<double>& T1,
                                                   const QVector<double>& T2,
                                                   const QVector<double>& F,
                                                   int m, int n) const
{
    QVector<double> Y(m * n, 0.0);

    for (int j = n - 1; j >= 0; --j) {
        /* 构造右端: rhs[i] = F[i][j] - sum_{k=j+1}^{n-1}(T2[j][k]*Y[i][k]) */
        QVector<double> rhs(m, 0.0);
        for (int i = 0; i < m; ++i) {
            rhs[i] = F[i * n + j];
            for (int k = j + 1; k < n; ++k) {
                rhs[i] -= T2[j * n + k] * Y[i * n + k];
            }
        }

        /* 求解 (T1 + T2[j][j]*I) * Y[:,j] = rhs */
        double diagB = T2[j * n + j];
        for (int i = m - 1; i >= 0; --i) {
            double sum = rhs[i];
            for (int k = i + 1; k < m; ++k) {
                sum -= T1[i * m + k] * Y[k * n + j];
            }
            double diagSum = T1[i * m + i] + diagB;
            Y[i * n + j] = (qFabs(diagSum) > 1e-14) ? sum / diagSum : 0.0;
        }
    }

    return Y;
}
