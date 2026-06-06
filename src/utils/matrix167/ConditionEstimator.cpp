/**
 * @file ConditionEstimator.cpp
 * @brief ConditionEstimator 实现
 *
 * 实现Hager 1-范数条件数估计：通过迭代求解 A^T*sign(Ax) 来逼近
 * ||A^{-1}||_1，结合||A||_1得到条件数kappa_1。
 */

#include "utils/matrix167/ConditionEstimator.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
ConditionEstimator::ConditionEstimator(QObject* parent)
    : QObject(parent)
{
}

ConditionEstimator::~ConditionEstimator() = default;

void ConditionEstimator::setMethod(Method method)
{
    m_method = method;
}

void ConditionEstimator::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(5, maxIter);
}

/**
 * @brief 计算矩阵1-范数(最大绝对列和)
 */
double ConditionEstimator::norm1(const QVector<QVector<double>>& matrix)
{
    if (matrix.isEmpty()) return 0.0;
    const int n = matrix.size();
    const int m = matrix[0].size();
    double maxColSum = 0.0;

    for (int j = 0; j < m; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < n; ++i) {
            colSum += qAbs(matrix[i][j]);
        }
        maxColSum = qMax(maxColSum, colSum);
    }

    return maxColSum;
}

/**
 * @brief 前向替换: Ly = Pb
 */
QVector<double> ConditionEstimator::forwardSolve(
    const QVector<QVector<double>>& lu,
    const QVector<int>& perm,
    const QVector<double>& b)
{
    const int n = lu.size();
    QVector<double> y(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = b[perm[i]];
        for (int j = 0; j < i; ++j) {
            sum -= lu[i][j] * y[j];
        }
        y[i] = sum;
    }

    return y;
}

/**
 * @brief 后向替换: Ux = y
 */
QVector<double> ConditionEstimator::backwardSolve(
    const QVector<QVector<double>>& lu,
    const QVector<double>& y)
{
    const int n = lu.size();
    QVector<double> x(n, 0.0);

    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= lu[i][j] * x[j];
        }
        if (qAbs(lu[i][i]) > 1e-15) {
            x[i] = sum / lu[i][i];
        }
    }

    return x;
}

/**
 * @brief Hager算法估计||A^{-1}||_1
 *
 * 迭代过程:
 * 1) x = 初始向量(全1)
 * 2) y = A^{-1} * x  (通过LU前向/后向替换)
 * 3) sign = sign(y)  (符号向量)
 * 4) z = A^{-T} * sign
 * 5) 找z中绝对值最大的分量j
 * 6) 若z[j] == x[j]对应的符号不变，收敛
 * 7) x = e_j(第j个单位向量)
 * 8) 跳回步骤2
 */
double ConditionEstimator::hagerNorm(const QVector<QVector<double>>& lu,
                                      const QVector<int>& perm)
{
    const int n = lu.size();
    if (n == 0) return 0.0;

    /* Initial vector: all ones */
    QVector<double> x(n, 1.0 / n);

    double estOld = 0.0;
    int iterations = 0;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        iterations++;

        /* Step 1: y = A^{-1} * x */
        QVector<double> y = forwardSolve(lu, perm, x);
        y = backwardSolve(lu, y);

        /* Compute current norm estimate */
        double est = 0.0;
        for (int i = 0; i < n; ++i) est += qAbs(y[i]);

        /* Check convergence */
        if (iter > 0 && qAbs(est - estOld) < estOld * 1e-8) {
            m_stats.hagerIterations += iterations;
            return est;
        }
        estOld = est;

        /* Step 2: sign vector */
        QVector<double> sign(n);
        for (int i = 0; i < n; ++i) {
            sign[i] = (y[i] >= 0.0) ? 1.0 : -1.0;
        }

        /* Step 3: z = A^{-T} * sign = (A^{-1})^T * sign */
        /* Solve U^T * w = sign first, then L^T * z = P * w */
        QVector<double> w(n, 0.0);
        for (int i = n - 1; i >= 0; --i) {
            double sum = sign[i];
            for (int j = i + 1; j < n; ++j) {
                sum -= lu[j][i] * w[j];   /* U^T: row j, col i */
            }
            if (qAbs(lu[i][i]) > 1e-15) {
                w[i] = sum / lu[i][i];
            }
        }

        /* L^T * z = P * w */
        QVector<double> z(n, 0.0);
        for (int i = 0; i < n; ++i) {
            z[i] = w[perm[i]];
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                z[i] -= lu[j][i] * z[j];  /* L^T: row j, col i */
            }
        }

        /* Step 4: find max absolute component */
        int jMax = 0;
        double maxVal = qAbs(z[0]);
        for (int i = 1; i < n; ++i) {
            if (qAbs(z[i]) > maxVal) {
                maxVal = qAbs(z[i]);
                jMax = i;
            }
        }

        /* Step 5: check if sign change would improve */
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double newVal = (i == jMax) ? 1.0 : 0.0;
            if (newVal != x[i]) changed = true;
        }

        /* New x = e_j */
        x.fill(0.0);
        x[jMax] = 1.0;

        if (!changed && iter > 0) {
            m_stats.hagerIterations += iterations;
            return est;
        }
    }

    m_stats.hagerIterations += iterations;
    return estOld;
}

/**
 * @brief 估计矩阵条件数
 *
 * 先做LU分解，再用Hager方法估计||A^{-1}||_1。
 * kappa_1 = ||A||_1 * ||A^{-1}||_1
 */
double ConditionEstimator::estimate(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = matrix.size();
    if (n == 0) return 1.0;

    /* Compute ||A||_1 */
    double aNorm = norm1(matrix);

    /* Perform LU decomposition in-place with partial pivoting */
    QVector<QVector<double>> lu = matrix;
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = i;

    for (int k = 0; k < n; ++k) {
        /* Find pivot */
        int maxRow = k;
        double maxVal = qAbs(lu[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(lu[i][k]) > maxVal) {
                maxVal = qAbs(lu[i][k]);
                maxRow = i;
            }
        }

        if (maxRow != k) {
            std::swap(lu[k], lu[maxRow]);
            std::swap(perm[k], perm[maxRow]);
        }

        if (qAbs(lu[k][k]) < 1e-15) continue;

        for (int i = k + 1; i < n; ++i) {
            lu[i][k] /= lu[k][k];
            for (int j = k + 1; j < n; ++j) {
                lu[i][j] -= lu[i][k] * lu[k][j];
            }
        }
    }

    double invNorm = hagerNorm(lu, perm);
    double condition = (aNorm > 1e-15) ? aNorm * invNorm : 1.0;

    m_stats.totalEstimates++;
    m_stats.lastConditionNumber = condition;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEstimates > 0)
        ? m_timeSum / m_stats.totalEstimates : 0.0;

    emit estimateCompleted(condition, m_stats.hagerIterations);
    return condition;
}

/**
 * @brief 从已有LU分解估计条件数
 */
double ConditionEstimator::estimateFromLU(const QVector<QVector<double>>& lu,
                                            const QVector<int>& perm)
{
    QElapsedTimer timer;
    timer.start();

    const int n = lu.size();
    if (n == 0) return 1.0;

    /* Estimate ||A||_1 from LU (reconstruct) */
    double aNorm = 0.0;
    for (int j = 0; j < n; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < n; ++i) {
            double val = 0.0;
            for (int k = 0; k <= qMin(i, j); ++k) {
                double l = (k == i) ? 1.0 : lu[i][k];
                val += l * lu[k][j];
            }
            colSum += qAbs(val);
        }
        aNorm = qMax(aNorm, colSum);
    }

    double invNorm = hagerNorm(lu, perm);
    double condition = (aNorm > 1e-15) ? aNorm * invNorm : 1.0;

    m_stats.totalEstimates++;
    m_stats.lastConditionNumber = condition;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEstimates > 0)
        ? m_timeSum / m_stats.totalEstimates : 0.0;

    emit estimateCompleted(condition, m_stats.hagerIterations);
    return condition;
}

/**
 * @brief 从SVD精确计算2-范数条件数
 */
double ConditionEstimator::conditionFromSVD(const QVector<double>& singularValues) const
{
    if (singularValues.isEmpty()) return 1.0;

    double sigmaMax = singularValues.first();
    double sigmaMin = singularValues.last();

    /* Find actual min (could have near-zero trailing values) */
    for (int i = singularValues.size() - 1; i >= 0; --i) {
        if (singularValues[i] > 1e-15) {
            sigmaMin = singularValues[i];
            break;
        }
    }

    return (sigmaMin > 1e-15) ? sigmaMax / sigmaMin : 1.0 / sigmaMin;
}

void ConditionEstimator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
