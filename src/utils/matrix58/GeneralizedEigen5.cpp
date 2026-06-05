/**
 * @file GeneralizedEigen5.cpp
 * @brief 广义特征值问题求解器实现，基于QZ分解
 *
 * 求解广义特征值问题 A*x = lambda*B*x，其中A和B为实方阵。
 * 使用QZ分解（广义Schur分解）方法：
 * 1. 同时将A和B化为上三角形式（通过正交变换）
 * 2. 从上三角矩阵对中提取广义特征值
 *
 * 适用于振动分析、结构力学、控制系统等领域。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix58/GeneralizedEigen5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化空矩阵
 * @param parent 父QObject对象指针
 */
GeneralizedEigen5::GeneralizedEigen5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行广义特征值分解
 *
 * 算法流程：
 * 1. 将A、B初始化为工作矩阵S、T
 * 2. 对(S, T)同时执行Hessenberg-三角化简
 * 3. 对化简后的矩阵对执行QZ迭代
 * 4. 从上三角结果中提取广义特征值
 *
 * @param A 矩阵A，行主序一维数组（n*n）
 * @param B 矩阵B，行主序一维数组（n*n）
 * @param n 矩阵维度
 * @return true分解成功
 */
bool GeneralizedEigen5::decompose(const QVector<double>& A, const QVector<double>& B, int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || A.size() < n * n || B.size() < n * n) return false;

    m_n = n;
    m_S = A;
    m_T = B;
    m_Q.resize(n * n, 0.0);
    m_Z.resize(n * n, 0.0);
    m_V.resize(n * n, 0.0);

    /* 初始化Q、Z为单位矩阵 */
    for (int i = 0; i < n; ++i) {
        m_Q[i * n + i] = 1.0;
        m_Z[i * n + i] = 1.0;
        m_V[i * n + i] = 1.0;
    }

    /* Step 1: 将B化简为上三角（右乘Givens旋转） */
    for (int j = 0; j < n; ++j) {
        for (int i = n - 1; i > j; --i) {
            double a = m_T[(i - 1) * n + j];
            double b = m_T[i * n + j];

            if (qAbs(b) < 1e-15) continue;

            double r = qSqrt(a * a + b * b);
            double c = a / r;
            double s = b / r;

            /* 对T和S执行行旋转 */
            for (int k = 0; k < n; ++k) {
                double t1 = m_T[(i - 1) * n + k];
                double t2 = m_T[i * n + k];
                m_T[(i - 1) * n + k] = c * t1 + s * t2;
                m_T[i * n + k] = -s * t1 + c * t2;

                double s1 = m_S[(i - 1) * n + k];
                double s2 = m_S[i * n + k];
                m_S[(i - 1) * n + k] = c * s1 + s * s2;
                m_S[i * n + k] = -s * s1 + c * s2;
            }

            /* 更新Q矩阵 */
            for (int k = 0; k < n; ++k) {
                double q1 = m_Q[k * n + (i - 1)];
                double q2 = m_Q[k * n + i];
                m_Q[k * n + (i - 1)] = c * q1 + s * q2;
                m_Q[k * n + i] = -s * q1 + c * q2;
            }
        }
    }

    /* Step 2: QZ迭代 */
    qzStep(m_S, m_T, n);

    /* 计算特征向量 V = Q * Z */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
                sum += m_Q[i * n + k] * m_Z[k * n + j];
            m_V[i * n + j] = sum;
        }
    }

    /* 更新统计 */
    m_stats.totalDecompositions++;
    m_stats.matrixSize = n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n);
    return true;
}

/**
 * @brief QZ迭代步骤
 *
 * 对矩阵对(S, T)执行隐式QZ迭代：
 * 1. 计算移位：S - mu*T 的右下2x2子矩阵特征值
 * 2. 对T执行Givens旋转保持上三角形式
 * 3. 对S执行相同旋转
 * 4. 迭代直到S接近上三角
 *
 * @param S 工作矩阵S（会被修改）
 * @param T 工作矩阵T（保持上三角）
 * @param n 矩阵维度
 */
void GeneralizedEigen5::qzStep(QVector<double>& S, QVector<double>& T, int n)
{
    const int maxIter = 100 * n;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 检查收敛性：S是否接近上三角 */
        bool converged = true;
        for (int i = 1; i < n && converged; ++i) {
            for (int j = 0; j < i; ++j) {
                if (qAbs(S[i * n + j]) > 1e-10) {
                    converged = false;
                    break;
                }
            }
        }
        if (converged) break;

        /* 对每对相邻行执行Givens旋转 */
        for (int i = 0; i < n - 1; ++i) {
            /* 计算S中的Givens旋转消除次对角元素 */
            double a = S[i * n + i];
            double b = S[(i + 1) * n + i];

            if (qAbs(b) < 1e-15) continue;

            double r = qSqrt(a * a + b * b);
            double c = a / r;
            double s = b / r;

            /* 左乘Givens旋转到S和T */
            for (int k = 0; k < n; ++k) {
                double s1 = S[i * n + k];
                double s2 = S[(i + 1) * n + k];
                S[i * n + k] = c * s1 + s * s2;
                S[(i + 1) * n + k] = -s * s1 + c * s2;

                double t1 = T[i * n + k];
                double t2 = T[(i + 1) * n + k];
                T[i * n + k] = c * t1 + s * t2;
                T[(i + 1) * n + k] = -s * t1 + c * t2;
            }

            /* 更新Q */
            for (int k = 0; k < n; ++k) {
                double q1 = m_Q[k * n + i];
                double q2 = m_Q[k * n + i + 1];
                m_Q[k * n + i] = c * q1 + s * q2;
                m_Q[k * n + i + 1] = -s * q1 + c * q2;
            }

            /* 恢复T的上三角形式：右乘Givens旋转 */
            for (int j = i; j < n - 1; ++j) {
                if (j + 1 < n) {
                    double a2 = T[j * n + j];
                    double b2 = T[(j + 1) * n + j];

                    if (qAbs(b2) < 1e-15) continue;

                    double r2 = qSqrt(a2 * a2 + b2 * b2);
                    double c2 = a2 / r2;
                    double s2 = b2 / r2;

                    /* 右乘旋转到T和S */
                    for (int k = 0; k < n; ++k) {
                        double t1 = T[k * n + j];
                        double t2 = T[k * n + j + 1];
                        T[k * n + j] = c2 * t1 + s2 * t2;
                        T[k * n + j + 1] = -s2 * t1 + c2 * t2;

                        double sv1 = S[k * n + j];
                        double sv2 = S[k * n + j + 1];
                        S[k * n + j] = c2 * sv1 + s2 * sv2;
                        S[k * n + j + 1] = -s2 * sv1 + c2 * sv2;
                    }

                    /* 更新Z */
                    for (int k = 0; k < n; ++k) {
                        double z1 = m_Z[k * n + j];
                        double z2 = m_Z[k * n + j + 1];
                        m_Z[k * n + j] = c2 * z1 + s2 * z2;
                        m_Z[k * n + j + 1] = -s2 * z1 + c2 * z2;
                    }
                }
            }
        }
    }

    /* 清理微小的次对角元素 */
    for (int i = 1; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            if (qAbs(S[i * n + j]) < 1e-10)
                S[i * n + j] = 0.0;
            if (j < i && qAbs(T[i * n + j]) < 1e-10)
                T[i * n + j] = 0.0;
        }
    }
}

/**
 * @brief 获取广义特征值
 *
 * 从QZ分解结果中提取广义特征值。
 * 对于上三角矩阵对(S, T)，特征值为 alpha/beta，
 * 其中 alpha = S[i][i], beta = T[i][i]。
 *
 * @return 特征值列表（实部, 虚部对）
 */
QVector<QPair<double, double>> GeneralizedEigen5::eigenvalues() const
{
    QVector<QPair<double, double>> eigs;
    if (m_n == 0) return eigs;

    for (int i = 0; i < m_n; ++i) {
        double alpha = m_S[i * m_n + i];
        double beta = m_T[i * m_n + i];

        if (qAbs(beta) < 1e-15) {
            /* 无穷大特征值 */
            eigs.append({1e15, 0.0});
        } else {
            double lambda = alpha / beta;
            eigs.append({lambda, 0.0});
        }
    }
    return eigs;
}

/**
 * @brief 重置所有统计数据
 */
void GeneralizedEigen5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
