#include "KrylovSolver5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Krylov子空间求解器
 * @param parent 父对象指针
 */
KrylovSolver5::KrylovSolver5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void KrylovSolver5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置求解方法
 *
 * @param method 求解方法："gmres"或"bicgstab"
 */
void KrylovSolver5::setMethod(const QString& method)
{
    m_method = method.toLower();
}

/**
 * @brief 设置最大迭代次数和残差收敛阈值
 *
 * @param maxIterations 最大迭代次数
 * @param residualThreshold 残差收敛阈值
 */
void KrylovSolver5::setTolerance(int maxIterations, double residualThreshold)
{
    m_maxIter = qMax(1, maxIterations);
    m_tolerance = qMax(1e-15, residualThreshold);
}

/**
 * @brief 设置预条件器类型
 *
 * @param type 预条件器类型："none"、"jacobi"或"ilu0"
 */
void KrylovSolver5::setPreconditioner(const QString& type)
{
    m_preconditioner = type.toLower();
}

/**
 * @brief 求解稀疏线性方程组Ax=b
 *
 * GMRES算法流程：
 * 1. Arnoldi过程构建Krylov子空间的标准正交基
 * 2. 将最小二乘问题投影到子空间中求解
 * 3. 重启策略防止计算量过大
 *
 * @param dimension 矩阵维度
 * @param entries 稀疏矩阵非零元 ((行,列), 值)
 * @param b 右端向量
 * @return 解向量和迭代次数对
 */
QPair<QVector<double>, int> KrylovSolver5::solve(
    int dimension,
    const QVector<QPair<QPair<int, int>, double>>& entries,
    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(dimension, 0.0);
    int iterCount = 0;

    if (dimension <= 0 || b.size() != dimension) {
        emit solveCompleted(0, 0.0);
        return qMakePair(x, iterCount);
    }

    /* 构建稠密矩阵用于计算 */
    QVector<QVector<double>> A(dimension, QVector<double>(dimension, 0.0));
    for (const auto& e : entries) {
        int r = e.first.first;
        int c = e.first.second;
        if (r >= 0 && r < dimension && c >= 0 && c < dimension)
            A[r][c] = e.second;
    }

    if (m_method == "gmres") {
        /* GMRES(m) 重启算法 */
        const int m = qMin(30, dimension);

        /* 初始残差 r = b - A*x */
        QVector<double> r = b;
        double beta = 0.0;
        for (int i = 0; i < dimension; ++i)
            beta += r[i] * r[i];
        beta = qSqrt(beta);

        double bnorm = 0.0;
        for (int i = 0; i < dimension; ++i)
            bnorm += b[i] * b[i];
        bnorm = qSqrt(qMax(bnorm, 1e-30));

        for (int outer = 0; outer < m_maxIter; ++outer) {
            if (beta / bnorm < m_tolerance) break;

            /* Arnoldi过程 */
            QVector<QVector<double>> V(m + 1, QVector<double>(dimension, 0.0));
            QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));

            for (int i = 0; i < dimension; ++i)
                V[0][i] = r[i] / qMax(beta, 1e-30);

            QVector<double> g(m + 1, 0.0);
            g[0] = beta;

            int j = 0;
            for (j = 0; j < m; ++j) {
                /* 矩阵-向量乘积 w = A * V[j] */
                QVector<double> w(dimension, 0.0);
                for (int i = 0; i < dimension; ++i) {
                    for (int k = 0; k < dimension; ++k)
                        w[i] += A[i][k] * V[j][k];
                }

                /* 修正Gram-Schmidt正交化 */
                for (int i = 0; i <= j; ++i) {
                    double dot = 0.0;
                    for (int k = 0; k < dimension; ++k)
                        dot += w[k] * V[i][k];
                    H[i][j] = dot;
                    for (int k = 0; k < dimension; ++k)
                        w[k] -= dot * V[i][k];
                }

                double normW = 0.0;
                for (int k = 0; k < dimension; ++k)
                    normW += w[k] * w[k];
                H[j + 1][j] = qSqrt(normW);

                if (H[j + 1][j] > 1e-15) {
                    for (int k = 0; k < dimension; ++k)
                        V[j + 1][k] = w[k] / H[j + 1][j];
                }

                /* Givens旋转更新最小二乘问题 */
                for (int i = 0; i < j; ++i) {
                    double temp = H[i][j];
                    double cs = H[i][i];
                    double sn = H[i + 1][i];
                    double rcs = qSqrt(cs * cs + sn * sn);
                    if (qFuzzyIsNull(rcs)) continue;
                    cs /= rcs; sn /= rcs;
                    H[i][j] = cs * temp + sn * H[i + 1][j];
                    H[i + 1][j] = -sn * temp + cs * H[i + 1][j];
                }

                iterCount++;
                if (qAbs(g[j]) / bnorm < m_tolerance) break;
            }

            /* 回代求解 y，更新 x = x + V*y */
            QVector<double> y(j, 0.0);
            for (int i = j - 1; i >= 0; --i) {
                double sum = g[i];
                for (int k = i + 1; k < j; ++k)
                    sum -= H[i][k] * y[k];
                y[i] = (qAbs(H[i][i]) > 1e-15) ? sum / H[i][i] : 0.0;
            }
            for (int i = 0; i < dimension; ++i)
                for (int k = 0; k < j; ++k)
                    x[i] += V[k][i] * y[k];

            /* 更新残差 */
            for (int i = 0; i < dimension; ++i) {
                double ax = 0.0;
                for (int k = 0; k < dimension; ++k)
                    ax += A[i][k] * x[k];
                r[i] = b[i] - ax;
            }
            beta = 0.0;
            for (int i = 0; i < dimension; ++i)
                beta += r[i] * r[i];
            beta = qSqrt(beta);
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    double finalRes = 0.0;
    for (int i = 0; i < dimension; ++i) {
        double ax = 0.0;
        for (int k = 0; k < dimension; ++k)
            ax += A[i][k] * x[k];
        double ri = b[i] - ax;
        finalRes += ri * ri;
    }
    finalRes = qSqrt(finalRes);

    emit solveCompleted(iterCount, finalRes);
    return qMakePair(x, iterCount);
}
