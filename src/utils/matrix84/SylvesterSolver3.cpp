/**
 * @file SylvesterSolver3.cpp
 * @brief Sylvester方程求解器实现 — Bartels-Stewart算法
 */

#include "utils/matrix84/SylvesterSolver3.h"

#include <QElapsedTimer>

#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
SylvesterSolver3::SylvesterSolver3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 求解Sylvester方程 AX + XB = C
 * @param A 左系数矩阵(m x m)
 * @param B 右系数矩阵(n x n)
 * @param C 右端矩阵(m x n)
 * @return 解矩阵X(m x n)
 */
QVector<QVector<double>> SylvesterSolver3::solve(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B,
    const QVector<QVector<double>>& C)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size();
    int n = B.size();
    if (m == 0 || n == 0) return {};

    /* --- Step 1: Schur分解 B = V * S * V^T --- */
    auto [S, V] = schurDecompose(B, n);

    /* --- Step 2: 变换右端 F = C * V --- */
    QVector<QVector<double>> F(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                F[i][j] += C[i][k] * V[k][j];
            }
        }
    }

    /* --- Step 3: 变换A: 直接使用A(此处简化, 对A做Schur分解更优) --- */
    /* 使用简化版本: 逐列求解 (A + s_ii * I) * y_i = f_i - sum */

    QVector<QVector<double>> Y(m, QVector<double>(n, 0.0));

    for (int j = 0; j < n; ++j) {
        /* 构造右端: rhs = F(:,j) - sum_{k=j+1}^{n-1} Y(:,k) * S(j,k) */
        QVector<double> rhs(m, 0.0);
        for (int i = 0; i < m; ++i) {
            rhs[i] = F[i][j];
            for (int k = j + 1; k < n; ++k) {
                rhs[i] -= Y[i][k] * S[j][k];
            }
        }

        /* 如果S有2x2块, 处理复共轭对 */
        if (j < n - 1 && std::abs(S[j + 1][j]) > 1e-14) {
            /* 2x2块: 联立求解第j和j+1列 */
            QVector<double> rhs2(m, 0.0);
            for (int i = 0; i < m; ++i) {
                rhs2[i] = F[i][j + 1];
                for (int k = j + 2; k < n; ++k) {
                    rhs2[i] -= Y[i][k] * S[j + 1][k];
                }
            }

            /* 2x2 Schur块的实部和虚部 */
            double s11 = S[j][j];
            double s12 = S[j][j + 1];
            double s21 = S[j + 1][j];
            double s22 = S[j + 1][j + 1];

            /* 对每行i求解2x2子系统 */
            /* [A + s11*I, s12*I] [Y_i_j  ]   [rhs_i  ]
             * [s21*I, A + s22*I] [Y_i_j+1] = [rhs2_i ] */
            for (int i = 0; i < m; ++i) {
                /* 简化: 标量情形直接求解2x2系统 */
                double aii = (i < A.size() && i < A[i].size()) ? A[i][i] : 0.0;
                double d1 = aii + s11;
                double d2 = aii + s22;

                double det = d1 * d2 - s12 * s21;
                if (std::abs(det) > 1e-15) {
                    Y[i][j]     = (d2 * rhs[i] - s12 * rhs2[i]) / det;
                    Y[i][j + 1] = (d1 * rhs2[i] - s21 * rhs[i]) / det;
                }
            }

            j++; /* 跳过下一个列 */
            continue;
        }

        /* 1x1块: 求解 (A + s_jj * I) * y = rhs */
        QVector<double> yCol = solveShiftedSystem(A, S[j][j], rhs, m);
        for (int i = 0; i < m; ++i) {
            Y[i][j] = yCol[i];
        }
    }

    /* --- Step 4: 反变换 X = Y * V^T --- */
    QVector<QVector<double>> X(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                X[i][j] += Y[i][k] * V[j][k]; /* V^T的元素 = V[k][j], 但用V[j][k]做简化 */
            }
        }
    }

    /* --- 更新统计 --- */
    m_stats.totalEquationsSolved++;
    m_stats.totalSize += m * n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEquationsSolved;

    double res = residual(X, A, B, C);
    emit equationSolved(m * n, res);
    return X;
}

/**
 * @brief 求解Lyapunov方程 AX + XA^T = C (A为稳定矩阵时唯一解)
 * @param A 系数矩阵(n x n)
 * @param C 右端矩阵(n x n, 通常为对称正定)
 * @return 解矩阵X
 */
QVector<QVector<double>> SylvesterSolver3::solveLyapunov(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& C)
{
    /* Lyapunov方程: AX + XA^T = C 等价于 B = A^T 的Sylvester方程 */
    int n = A.size();
    QVector<QVector<double>> AT(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            AT[i][j] = A[j][i];
        }
    }
    return solve(A, AT, C);
}

/**
 * @brief 计算Sylvester方程解的残差 ||AX + XB - C||_F
 * @param X 候选解矩阵
 * @param A 左系数矩阵
 * @param B 右系数矩阵
 * @param C 右端矩阵
 * @return Frobenius范数残差
 */
double SylvesterSolver3::residual(const QVector<QVector<double>>& X,
                                   const QVector<QVector<double>>& A,
                                   const QVector<QVector<double>>& B,
                                   const QVector<QVector<double>>& C) const
{
    int m = X.size();
    if (m == 0) return 0.0;
    int n = X[0].size();

    double norm = 0.0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            /* (AX + XB - C)[i][j] */
            double r = -C[i][j];
            for (int k = 0; k < m; ++k) {
                r += A[i][k] * X[k][j]; /* AX的第i行j列 */
            }
            for (int k = 0; k < n; ++k) {
                r += X[i][k] * B[k][j]; /* XB的第i行j列 */
            }
            norm += r * r;
        }
    }
    return std::sqrt(norm);
}

/** @brief 重置统计信息 */
void SylvesterSolver3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief Schur分解(简化QR迭代) — 内部辅助方法
 * @param mat 输入矩阵
 * @param n 维度
 * @return {T(上三角), Q(正交)}对
 */
std::pair<QVector<QVector<double>>, QVector<QVector<double>>>
SylvesterSolver3::schurDecompose(const QVector<QVector<double>>& mat, int n) const
{
    QVector<QVector<double>> T = mat;
    QVector<QVector<double>> Q(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) Q[i][i] = 1.0;

    /* Hessenberg化简 */
    for (int k = 0; k < n - 2; ++k) {
        /* 计算Householder向量 */
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i) sigma += T[i][k] * T[i][k];

        if (sigma < 1e-30 && std::abs(T[k + 1][k]) < 1e-30) continue;

        double alpha = T[k + 1][k];
        double normX = std::sqrt(alpha * alpha + sigma);
        if (normX < 1e-30) continue;

        double v0 = (alpha >= 0) ? alpha + normX : alpha - normX;
        QVector<double> v(n - k - 1, 0.0);
        v[0] = v0;
        for (int i = 1; i < n - k - 1; ++i) v[i] = T[k + 1 + i][k];

        double vNormSq = v0 * v0 + sigma;
        if (vNormSq < 1e-30) continue;
        double beta = -2.0 / vNormSq;

        /* 应用Householder反射: 左乘 */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < (int)v.size(); ++i)
                dot += v[i] * T[k + 1 + i][j];
            for (int i = 0; i < (int)v.size(); ++i)
                T[k + 1 + i][j] += beta * v[i] * dot;
        }

        /* 右乘 */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < (int)v.size(); ++j)
                dot += T[i][k + 1 + j] * v[j];
            for (int j = 0; j < (int)v.size(); ++j)
                T[i][k + 1 + j] += beta * dot * v[j];
        }

        /* 累积到Q */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < (int)v.size(); ++j)
                dot += Q[i][k + 1 + j] * v[j];
            for (int j = 0; j < (int)v.size(); ++j)
                Q[i][k + 1 + j] += beta * dot * v[j];
        }
    }

    /* 简化QR迭代(最大50*n步) */
    int p = n - 1;
    int totalIter = 0;
    while (p > 0 && totalIter < 50 * n) {
        double sp = std::abs(T[p][p - 1]);
        double ap = std::abs(T[p - 1][p - 1]) + std::abs(T[p][p]);
        if (sp <= 1e-14 * ap || sp < 1e-30) {
            T[p][p - 1] = 0.0;
            p--;
            continue;
        }

        /* Wilkinson位移 */
        double a = T[p - 1][p - 1], b = T[p - 1][p];
        double c = T[p][p - 1], d = T[p][p];
        double s = a + d;
        double t = a * d - b * c;

        /* 单步隐式QR: Givens旋转 */
        for (int k = 0; k < p; ++k) {
            double x_val = T[k][k] * T[k][k] + ((k + 1 < n) ? T[k][k + 1] * T[k + 1][k] : 0.0) - s * T[k][k] + t;
            double y_val = ((k + 1 < n) ? T[k + 1][k] : 0.0) * (T[k][k] + ((k + 1 < n) ? T[k + 1][k + 1] : 0.0) - s);

            double r = std::sqrt(x_val * x_val + y_val * y_val);
            if (r < 1e-30) continue;
            double cs = x_val / r;
            double sn = y_val / r;

            /* Givens旋转: 左乘 */
            for (int j = 0; j < n; ++j) {
                double t1 = T[k][j], t2 = T[k + 1][j];
                T[k][j] = cs * t1 + sn * t2;
                T[k + 1][j] = -sn * t1 + cs * t2;
            }
            /* 右乘 */
            for (int j = 0; j < n; ++j) {
                double t1 = T[j][k], t2 = T[j][k + 1];
                T[j][k] = cs * t1 + sn * t2;
                T[j][k + 1] = -sn * t1 + cs * t2;
            }
            /* 累积Q */
            for (int j = 0; j < n; ++j) {
                double t1 = Q[j][k], t2 = Q[j][k + 1];
                Q[j][k] = cs * t1 + sn * t2;
                Q[j][k + 1] = -sn * t1 + cs * t2;
            }
        }
        totalIter++;
    }

    return {T, Q};
}

/**
 * @brief 求解带移位的线性系统 (A + sigma*I)*x = b — 内部辅助方法
 * 使用LU分解求解, A为m x m矩阵
 * @param A 系数矩阵
 * @param sigma 移位量
 * @param b 右端向量
 * @param m 维度
 * @return 解向量
 */
QVector<double> SylvesterSolver3::solveShiftedSystem(
    const QVector<QVector<double>>& A, double sigma,
    const QVector<double>& b, int m) const
{
    /* 构造 (A + sigma*I) */
    QVector<QVector<double>> M(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            M[i][j] = A[i][j];
        }
        M[i][i] += sigma;
    }

    /* LU分解 */
    QVector<int> piv(m);
    for (int i = 0; i < m; ++i) piv[i] = i;

    for (int k = 0; k < m; ++k) {
        int maxRow = k;
        double maxVal = std::abs(M[k][k]);
        for (int i = k + 1; i < m; ++i) {
            if (std::abs(M[i][k]) > maxVal) {
                maxVal = std::abs(M[i][k]);
                maxRow = i;
            }
        }
        if (maxRow != k) {
            std::swap(M[k], M[maxRow]);
            std::swap(piv[k], piv[maxRow]);
        }
        if (std::abs(M[k][k]) < 1e-15) continue;

        for (int i = k + 1; i < m; ++i) {
            M[i][k] /= M[k][k];
            for (int j = k + 1; j < m; ++j) {
                M[i][j] -= M[i][k] * M[k][j];
            }
        }
    }

    /* 求解 */
    QVector<double> x = b;
    for (int i = 0; i < m; ++i) {
        if (piv[i] != i) std::swap(x[i], x[piv[i]]);
    }
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < i; ++j) {
            x[i] -= M[i][j] * x[j];
        }
    }
    for (int i = m - 1; i >= 0; --i) {
        for (int j = i + 1; j < m; ++j) {
            x[i] -= M[i][j] * x[j];
        }
        if (std::abs(M[i][i]) > 1e-15) x[i] /= M[i][i];
    }

    return x;
}
