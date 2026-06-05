/**
 * @file SingularValueSolver.cpp
 * @brief SingularValueSolver 实现
 *
 * 实现精简SVD：Golub-Kahan双对角化 + 隐式QR位移迭代。
 */

#include "utils/matrix165/SingularValueSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
SingularValueSolver::SingularValueSolver(QObject* parent)
    : QObject(parent)
{
}

void SingularValueSolver::setEpsilon(double eps)
{
    m_eps = qMax(1e-16, eps);
}

void SingularValueSolver::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(10, maxIter);
}

/**
 * @brief 计算Householder向量
 *
 * 返回将x[start:]映射到||x||e_1方向的Householder变换系数。
 */
QVector<double> SingularValueSolver::householderVector(
    const QVector<double>& x, int start) const
{
    int n = x.size() - start;
    if (n <= 0) return QVector<double>();

    double sigma = 0.0;
    for (int i = start + 1; i < x.size(); ++i) {
        sigma += x[i] * x[i];
    }

    QVector<double> v(n);
    v[0] = x[start];
    for (int i = 1; i < n; ++i) {
        v[i] = x[start + i];
    }

    double normV = qSqrt(v[0] * v[0] + sigma);
    if (normV < m_eps) return QVector<double>(n, 0.0);

    if (v[0] >= 0) v[0] += normV;
    else v[0] -= normV;

    double normV2 = qSqrt(v[0] * v[0] + sigma);
    if (normV2 < m_eps) return QVector<double>(n, 0.0);

    for (auto& vi : v) vi /= normV2;

    return v;
}

/**
 * @brief Golub-Kahan双对角化
 *
 * 将m×n矩阵A约化为双对角形式：B = U_L^T A U_R
 * diagonal = B的主对角线，superDiagonal = B的上对角线
 */
void SingularValueSolver::bidiagonalize(QVector<QVector<double>>& A,
                                        QVector<double>& diagonal,
                                        QVector<double>& superDiagonal)
{
    const int m = A.size();
    const int n = (m > 0) ? A[0].size() : 0;
    const int k = qMin(m, n);

    diagonal.resize(k);
    superDiagonal.resize(k);

    for (int i = 0; i < k; ++i) {
        /* 左Householder: 消去A[i+1:m, i] */
        QVector<double> col(m - i);
        for (int j = i; j < m; ++j) col[j - i] = A[j][i];

        QVector<double> vLeft = householderVector(col, 0);

        /* A = (I - 2vv^T) A */
        for (int jj = i; jj < n; ++jj) {
            double dot = 0.0;
            for (int ii = 0; ii < vLeft.size(); ++ii) {
                dot += vLeft[ii] * A[i + ii][jj];
            }
            for (int ii = 0; ii < vLeft.size(); ++ii) {
                A[i + ii][jj] -= 2.0 * vLeft[ii] * dot;
            }
        }

        diagonal[i] = A[i][i];

        /* 右Householder: 消去A[i, i+2:n] */
        if (i < n - 1) {
            QVector<double> row(n - i - 1);
            for (int jj = i + 1; jj < n; ++jj) row[jj - i - 1] = A[i][jj];

            QVector<double> vRight = householderVector(row, 0);

            /* A = A (I - 2vv^T) */
            for (int ii = i; ii < m; ++ii) {
                double dot = 0.0;
                for (int jj = 0; jj < vRight.size(); ++jj) {
                    dot += vRight[jj] * A[ii][i + 1 + jj];
                }
                for (int jj = 0; jj < vRight.size(); ++jj) {
                    A[ii][i + 1 + jj] -= 2.0 * vRight[jj] * dot;
                }
            }

            superDiagonal[i] = (i + 1 < n) ? A[i][i + 1] : 0.0;
        }
    }
}

/**
 * @brief Givens旋转参数
 */
void SingularValueSolver::givensRotation(double a, double b, double& c, double& s) const
{
    if (qAbs(b) < m_eps) {
        c = 1.0;
        s = 0.0;
    } else if (qAbs(b) > qAbs(a)) {
        double t = -a / b;
        s = 1.0 / qSqrt(1.0 + t * t);
        c = s * t;
    } else {
        double t = -b / a;
        c = 1.0 / qSqrt(1.0 + t * t);
        s = c * t;
    }
}

/**
 * @brief 隐式QR位移迭代
 *
 * 对双对角矩阵执行隐式QR位移，收敛后得到奇异值。
 */
int SingularValueSolver::qrShiftIteration(QVector<double>& diagonal,
                                          QVector<double>& superDiagonal,
                                          QVector<QVector<double>>& U,
                                          QVector<QVector<double>>& V)
{
    const int n = diagonal.size();
    int iterations = 0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* 检查收敛 */
        bool converged = true;
        for (int i = 0; i < n - 1; ++i) {
            if (qAbs(superDiagonal[i]) > m_eps * (qAbs(diagonal[i]) + qAbs(diagonal[i + 1]))) {
                converged = false;
                break;
            }
        }
        if (converced) break;

        /* 找未收敛的子矩阵 */
        int q = n - 1;
        while (q > 0 && qAbs(superDiagonal[q - 1]) <= m_eps * (qAbs(diagonal[q - 1]) + qAbs(diagonal[q]))) {
            q--;
        }
        int p = q - 1;
        while (p > 0 && qAbs(superDiagonal[p - 1]) > m_eps * (qAbs(diagonal[p - 1]) + qAbs(diagonal[p]))) {
            p--;
        }

        /* Wilkinson位移 */
        double d = (diagonal[q - 1] - diagonal[q]) / 2.0;
        double mu = diagonal[q] - superDiagonal[q - 1] * superDiagonal[q - 1]
            / (d + (d >= 0 ? 1 : -1) * qSqrt(qMax(0.0, d * d + superDiagonal[q - 1] * superDiagonal[q - 1])));

        /* 隐式QR步(Givens旋转追赶) */
        double x = diagonal[p] - mu;
        double z = superDiagonal[p];

        for (int k = p; k < q; ++k) {
            double c, s;
            givensRotation(x, z, c, s);

            /* 更新双对角矩阵 */
            double w1 = c * diagonal[k] - s * superDiagonal[k];
            double w2 = s * diagonal[k] + c * superDiagonal[k];
            double w3 = (k + 1 < n) ? s * diagonal[k + 1] : 0.0;
            double w4 = (k + 1 < n) ? c * diagonal[k + 1] : 0.0;

            diagonal[k] = c * w1 - s * w3;
            if (k + 1 < n) {
                superDiagonal[k] = c * w2 + s * w4;
                diagonal[k + 1] = -s * w2 + c * w4;
            }

            /* 更新V */
            for (int i = 0; i < V.size(); ++i) {
                double v1 = V[i][k];
                double v2 = V[i][k + 1];
                V[i][k] = c * v1 - s * v2;
                V[i][k + 1] = s * v1 + c * v2;
            }

            if (k + 1 < q) {
                x = superDiagonal[k];
                z = (k + 2 < n) ? s * superDiagonal[k + 1] : 0.0;
                if (k + 2 < n) superDiagonal[k + 1] *= c;
            }
        }

        iterations++;
    }

    return iterations;
}

/**
 * @brief 计算精简SVD
 */
SingularValueSolver::SVDResult SingularValueSolver::decompose(
    const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    SVDResult result;
    const int m = A.size();
    if (m == 0) return result;
    const int n = A[0].size();
    if (n == 0) return result;

    m_stats.matrixRows = m;
    m_stats.matrixCols = n;

    /* 复制矩阵 */
    QVector<QVector<double>> B = A;

    /* 双对角化 */
    QVector<double> diagonal, superDiagonal;
    bidiagonalize(B, diagonal, superDiagonal);

    const int k = qMin(m, n);

    /* 初始化U和V为单位矩阵 */
    result.U.resize(m);
    for (int i = 0; i < m; ++i) {
        result.U[i].resize(k, 0.0);
        if (i < k) result.U[i][i] = 1.0;
    }

    result.V.resize(n);
    for (int i = 0; i < n; ++i) {
        result.V[i].resize(k, 0.0);
        if (i < k) result.V[i][i] = 1.0;
    }

    /* QR位移迭代 */
    int iters = qrShiftIteration(diagonal, superDiagonal, result.U, result.V);

    /* 奇异值取绝对值并降序排列 */
    result.singularValues.resize(k);
    for (int i = 0; i < k; ++i) {
        result.singularValues[i] = qAbs(diagonal[i]);
    }

    /* 降序排序 */
    QVector<int> indices(k);
    for (int i = 0; i < k; ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        return result.singularValues[a] > result.singularValues[b];
    });

    QVector<double> sortedSV(k);
    for (int i = 0; i < k; ++i) sortedSV[i] = result.singularValues[indices[i]];
    result.singularValues = sortedSV;

    /* 条件数 */
    double svMax = result.singularValues[0];
    double svMin = result.singularValues[k - 1];
    m_stats.conditionNumber = (svMin > m_eps) ? svMax / svMin : 1e15;

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions > 0)
        ? m_timeSum / m_stats.totalDecompositions : 0.0;

    emit decompositionCompleted(m, n, iters);
    return result;
}

QVector<double> SingularValueSolver::singularValuesOnly(
    const QVector<QVector<double>>& A)
{
    SVDResult res = decompose(A);
    return res.singularValues;
}

void SingularValueSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
