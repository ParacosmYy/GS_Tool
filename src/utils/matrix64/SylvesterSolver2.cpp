/**
 * @file SylvesterSolver2.cpp
 * @brief Sylvester方程求解器实现 (Bartels-Stewart算法)
 *
 * 求解Sylvester矩阵方程: AX + XB = C
 * 其中A为m×m矩阵，B为n×n矩阵，C和X为m×n矩阵。
 * 使用Bartels-Stewart算法:
 * 1. 将A和B分别约化为上Schur形式 (上三角矩阵)
 * 2. 通过前向/后向代入求解简化后的方程
 * 适用于连续时间系统控制和信号处理中的矩阵方程。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/matrix64/SylvesterSolver2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化Sylvester方程求解器
 * @param parent 父QObject指针
 */
SylvesterSolver2::SylvesterSolver2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置Sylvester方程的矩阵
 * @param A 左侧矩阵 (m×m)
 * @param B 右侧矩阵 (n×n)
 * @param C 常数矩阵 (m×n)
 *
 * 方程形式: AX + XB = C
 */
void SylvesterSolver2::setMatrices(const QVector<QVector<double>>& A,
                                     const QVector<QVector<double>>& B,
                                     const QVector<QVector<double>>& C)
{
    m_A = A;
    m_B = B;
    m_C = C;
    m_n = A.size();
    m_solved = false;
    m_residual = 0.0;
}

/**
 * @brief 求解Sylvester方程
 *
 * 调用Bartels-Stewart算法求解 AX + XB = C
 *
 * @return 解矩阵X (m×n)
 */
QVector<QVector<double>> SylvesterSolver2::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_A.isEmpty() || m_B.isEmpty() || m_C.isEmpty()) {
        m_solved = false;
        m_residual = 0.0;
        emit solveCompleted(0, 0.0);
        return QVector<QVector<double>>();
    }

    int m = m_A.size();
    int n = m_B.size();

    /* 验证矩阵维度 */
    for (int i = 0; i < m; ++i) {
        if (m_A[i].size() != m || m_C[i].size() != n) {
            m_solved = false;
            emit solveCompleted(0, 0.0);
            return QVector<QVector<double>>();
        }
    }
    for (int i = 0; i < n; ++i) {
        if (m_B[i].size() != n) {
            m_solved = false;
            emit solveCompleted(0, 0.0);
            return QVector<QVector<double>>();
        }
    }

    QVector<QVector<double>> X = bartelsStewart();

    /* 计算残差: ||AX + XB - C||_F */
    m_residual = 0.0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double val = -m_C[i][j];
            for (int k = 0; k < m; ++k) {
                val += m_A[i][k] * X[k][j];
            }
            for (int k = 0; k < n; ++k) {
                val += X[i][k] * m_B[k][j];
            }
            m_residual += val * val;
        }
    }
    m_residual = qSqrt(m_residual);
    m_solved = true;

    /* 更新统计 */
    m_stats.totalSolves++;
    m_stats.totalDimensions += m + n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m + n, m_residual);
    return X;
}

/**
 * @brief 重置所有统计数据
 */
void SylvesterSolver2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Bartels-Stewart算法核心实现
 *
 * 步骤:
 * 1. 使用Givens旋转变换A和B为上三角形式
 * 2. 对C做相同变换得到F
 * 3. 求解上三角系统: A~Y + YB~ = F
 * 4. 对Y做逆变换得到X
 *
 * 对于简化实现，我们使用:
 * 1. QR分解将A和B约化为上三角
 * 2. 求解简化后的三角形Sylvester方程
 *
 * @return 解矩阵X
 */
QVector<QVector<double>> SylvesterSolver2::bartelsStewart()
{
    int m = m_A.size();
    int n = m_B.size();

    /* 步骤1: 将A约化为上三角形式 (简化QR) */
    QVector<QVector<double>> R_A = m_A;
    QVector<QVector<double>> Q_A(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) Q_A[i][i] = 1.0;

    for (int col = 0; col < m - 1; ++col) {
        for (int row = m - 1; row > col; --row) {
            double a = R_A[row - 1][col];
            double b = R_A[row][col];
            double r = qSqrt(a * a + b * b);
            if (r < 1e-15) continue;

            double c = a / r;
            double s = -b / r;

            /* 旋转R_A的行 */
            for (int j = 0; j < m; ++j) {
                double tmp1 = c * R_A[row - 1][j] - s * R_A[row][j];
                double tmp2 = s * R_A[row - 1][j] + c * R_A[row][j];
                R_A[row - 1][j] = tmp1;
                R_A[row][j] = tmp2;
            }

            /* 更新Q_A */
            for (int j = 0; j < m; ++j) {
                double tmp1 = c * Q_A[j][row - 1] - s * Q_A[j][row];
                double tmp2 = s * Q_A[j][row - 1] + c * Q_A[j][row];
                Q_A[j][row - 1] = tmp1;
                Q_A[j][row] = tmp2;
            }
        }
    }

    /* 步骤2: 将B约化为上三角形式 */
    QVector<QVector<double>> R_B = m_B;
    QVector<QVector<double>> Q_B(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) Q_B[i][i] = 1.0;

    for (int col = 0; col < n - 1; ++col) {
        for (int row = n - 1; row > col; --row) {
            double a = R_B[row - 1][col];
            double b = R_B[row][col];
            double r = qSqrt(a * a + b * b);
            if (r < 1e-15) continue;

            double c = a / r;
            double s = -b / r;

            for (int j = 0; j < n; ++j) {
                double tmp1 = c * R_B[row - 1][j] - s * R_B[row][j];
                double tmp2 = s * R_B[row - 1][j] + c * R_B[row][j];
                R_B[row - 1][j] = tmp1;
                R_B[row][j] = tmp2;
            }

            for (int j = 0; j < n; ++j) {
                double tmp1 = c * Q_B[j][row - 1] - s * Q_B[j][row];
                double tmp2 = s * Q_B[j][row - 1] + c * Q_B[j][row];
                Q_B[j][row - 1] = tmp1;
                Q_B[j][row] = tmp2;
            }
        }
    }

    /* 步骤3: 变换C -> F = Q_A^T * C * Q_B */
    /* 先计算 C * Q_B */
    QVector<QVector<double>> CQB(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                CQB[i][j] += m_C[i][k] * Q_B[k][j];
            }
        }
    }

    /* 再计算 Q_A^T * CQB */
    QVector<QVector<double>> F(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < m; ++k) {
                F[i][j] += Q_A[k][i] * CQB[k][j];
            }
        }
    }

    /* 步骤4: 求解上三角Sylvester方程: R_A * Y + Y * R_B = F */
    QVector<QVector<double>> Y(m, QVector<double>(n, 0.0));

    /* 从右下角开始，逐列求解 */
    for (int j = n - 1; j >= 0; --j) {
        for (int i = m - 1; i >= 0; --i) {
            double sum = F[i][j];

            /* 减去 R_A[i,k] * Y[k,j] (k > i) */
            for (int k = i + 1; k < m; ++k) {
                sum -= R_A[i][k] * Y[k][j];
            }

            /* 减去 Y[i,k] * R_B[k,j] (k > j) */
            for (int k = j + 1; k < n; ++k) {
                sum -= Y[i][k] * R_B[k][j];
            }

            /* 对角元素: R_A[i,i] + R_B[j,j] */
            double diag = R_A[i][i] + R_B[j][j];
            if (qAbs(diag) < 1e-15) {
                Y[i][j] = 0.0; /* 奇异情况 */
            } else {
                Y[i][j] = sum / diag;
            }
        }
    }

    /* 步骤5: 逆变换 X = Q_A * Y * Q_B^T */
    /* 先计算 Y * Q_B^T */
    QVector<QVector<double>> YQBT(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                YQBT[i][j] += Y[i][k] * Q_B[j][k];
            }
        }
    }

    /* 再计算 Q_A * YQBT */
    QVector<QVector<double>> X(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < m; ++k) {
                X[i][j] += Q_A[i][k] * YQBT[k][j];
            }
        }
    }

    return X;
}
