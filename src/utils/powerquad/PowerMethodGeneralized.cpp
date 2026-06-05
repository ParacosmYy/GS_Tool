/**
 * @file PowerMethodGeneralized.cpp
 * @brief 广义幂法实现 — 广义特征值问题 Ax = λBx
 */

#include "utils/powerquad/PowerMethodGeneralized.h"

#include <QElapsedTimer>
#include <QtMath>

#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
PowerMethodGeneralized::PowerMethodGeneralized(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 求解最大广义特征值 */
QPair<double, QVector<double>> PowerMethodGeneralized::solve(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B,
    int maxIter,
    double tol)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0 || B.size() == 0) return {0.0, {}};

    /* 检查维度一致性 */
    for (int i = 0; i < n; ++i) {
        if (A[i].size() != n || B[i].size() != n) {
            return {0.0, {}};
        }
    }

    /* 初始向量: 全1 */
    QVector<double> x(n, 1.0 / qSqrt(static_cast<double>(n)));

    double eigenvalue = 0.0;
    int iter = 0;

    for (; iter < maxIter; ++iter) {
        /* 求解 B * z = A * x (通过Cholesky分解) */
        QVector<double> Ax = matVecMul(A, x);
        QVector<double> z = choleskySolve(B, Ax);

        /* Rayleigh商: λ = x^T A x / x^T B x */
        QVector<double> Bx = matVecMul(B, x);
        double xTAx = dot(x, Ax);
        double xTBx = dot(x, Bx);

        double newEigenvalue = (qAbs(xTBx) > 1e-30) ? xTAx / xTBx : 0.0;

        /* 归一化 z */
        double zNorm = vecNorm(z);
        if (zNorm < 1e-30) break;

        for (int i = 0; i < n; ++i) {
            z[i] /= zNorm;
        }

        /* 收敛判断 */
        if (iter > 0 && qAbs(newEigenvalue - eigenvalue) < tol) {
            eigenvalue = newEigenvalue;
            x = z;
            ++iter;
            break;
        }

        eigenvalue = newEigenvalue;
        x = z;
    }

    /* 最终Rayleigh商精化 */
    QVector<double> Ax = matVecMul(A, x);
    QVector<double> Bx = matVecMul(B, x);
    double xTAx = dot(x, Ax);
    double xTBx = dot(x, Bx);
    if (qAbs(xTBx) > 1e-30) {
        eigenvalue = xTAx / xTBx;
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(eigenvalue, iter);
    return {eigenvalue, x};
}

/** @brief 矩阵-向量乘积 */
QVector<double> PowerMethodGeneralized::matVecMul(
    const QVector<QVector<double>>& mat, const QVector<double>& v)
{
    int n = mat.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int cols = qMin(static_cast<int>(mat[i].size()), n);
        for (int j = 0; j < cols; ++j) {
            result[i] += mat[i][j] * v[j];
        }
    }
    return result;
}

/** @brief 向量点积 */
double PowerMethodGeneralized::dot(const QVector<double>& a,
                                   const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/** @brief 向量范数 */
double PowerMethodGeneralized::vecNorm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/** @brief Cholesky分解求解对称正定系统
 *  @param mat 对称正定矩阵
 *  @param rhs 右端向量
 *  @return 解向量 */
QVector<double> PowerMethodGeneralized::choleskySolve(
    const QVector<QVector<double>>& mat, const QVector<double>& rhs)
{
    int n = mat.size();
    if (n == 0) return {};

    /* Cholesky分解: mat = L * L^T */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double sum = mat[i][j];
            for (int k = 0; k < j; ++k) {
                sum -= L[i][k] * L[j][k];
            }

            if (i == j) {
                /* 对角元素 */
                if (sum <= 0.0) {
                    /* 非正定: 添加正则化 */
                    sum = 1e-10;
                }
                L[i][j] = qSqrt(sum);
            } else {
                L[i][j] = sum / (qAbs(L[j][j]) > 1e-30 ? L[j][j] : 1e-15);
            }
        }
    }

    /* 前代: L * y = rhs */
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = rhs[i];
        for (int k = 0; k < i; ++k) {
            sum -= L[i][k] * y[k];
        }
        y[i] = sum / (qAbs(L[i][i]) > 1e-30 ? L[i][i] : 1e-15);
    }

    /* 回代: L^T * x = y */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int k = i + 1; k < n; ++k) {
            sum -= L[k][i] * x[k];
        }
        x[i] = sum / (qAbs(L[i][i]) > 1e-30 ? L[i][i] : 1e-15);
    }

    return x;
}

/** @brief 重置统计 */
void PowerMethodGeneralized::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
