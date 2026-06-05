/**
 * @file SymmetricEigen4.cpp
 * @brief 对称特征值增强实现 — Jacobi/三对角QR/二分法/分治
 */

#include "utils/matrix30/SymmetricEigen4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SymmetricEigen4::SymmetricEigen4(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置收敛阈值 @param tol 容差 */
void SymmetricEigen4::setTolerance(double tol)
{
    m_tolerance = qMax(1e-15, tol);
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void SymmetricEigen4::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(10, maxIter);
}

/** @brief 特征分解 @param matrix 对称矩阵 @param n 阶数 @param algo 算法 @return 结果 */
SymmetricEigen4::EigenResult SymmetricEigen4::decompose(
    const QVector<double>& matrix, int n, Algorithm algo)
{
    QElapsedTimer timer;
    timer.start();

    EigenResult result;
    if (n <= 0 || matrix.size() < n * n) return result;

    switch (algo) {
    case Algorithm::Jacobi:
        result = jacobiMethod(matrix, n);
        break;
    case Algorithm::TridiagonalQR:
        result = tridiagQRMethod(matrix, n);
        break;
    case Algorithm::Bisection:
        result = bisectionMethod(matrix, n);
        break;
    case Algorithm::DivideConquer:
        result = divideConquerMethod(matrix, n);
        break;
    }

    double elapsed = timer.elapsed();
    m_stats.totalDecompositions++;
    m_stats.totalElementsProcessed += static_cast<quint64>(n * n);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecompositions);
    if (result.converged) m_stats.totalConverged++;

    emit decomposeComplete(n, result.converged);
    return result;
}

/** @brief Jacobi旋转法 @param matrix 矩阵 @param n 阶数 @return 结果 */
SymmetricEigen4::EigenResult SymmetricEigen4::jacobiMethod(
    const QVector<double>& matrix, int n)
{
    EigenResult result;
    QVector<double> A = matrix;

    /* 初始化特征向量为单位矩阵 */
    QVector<double> V(n * n, 0.0);
    for (int i = 0; i < n; ++i) V[i * n + i] = 1.0;

    int iter = 0;
    for (iter = 0; iter < m_maxIterations; ++iter) {
        /* 找最大非对角元素 */
        double maxOff = 0.0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double val = qAbs(A[i * n + j]);
                if (val > maxOff) {
                    maxOff = val;
                    p = i; q = j;
                }
            }
        }

        if (maxOff < m_tolerance) {
            result.converged = true;
            break;
        }

        /* 计算旋转角 */
        double app = A[p * n + p], aqq = A[q * n + q], apq = A[p * n + q];
        double theta = 0.0;
        if (qFuzzyCompare(qAbs(app - aqq), 0.0)) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * qAtan2(2.0 * apq, app - aqq);
        }
        double c = qCos(theta), s = qSin(theta);

        /* 应用Givens旋转 */
        for (int i = 0; i < n; ++i) {
            if (i == p || i == q) continue;
            double aip = A[i * n + p], aiq = A[i * n + q];
            A[i * n + p] = c * aip + s * aiq;
            A[p * n + i] = A[i * n + p];
            A[i * n + q] = -s * aip + c * aiq;
            A[q * n + i] = A[i * n + q];
        }
        double newPP = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        double newQQ = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        A[p * n + p] = newPP;
        A[q * n + q] = newQQ;
        A[p * n + q] = 0.0;
        A[q * n + p] = 0.0;

        /* 累积特征向量 */
        for (int i = 0; i < n; ++i) {
            double vip = V[i * n + p], viq = V[i * n + q];
            V[i * n + p] = c * vip + s * viq;
            V[i * n + q] = -s * vip + c * viq;
        }
    }

    result.iterations = iter;
    /* 提取特征值 */
    result.eigenvalues.resize(n);
    for (int i = 0; i < n; ++i) result.eigenvalues[i] = A[i * n + i];

    /* 提取特征向量 */
    result.eigenvectors.resize(n);
    for (int j = 0; j < n; ++j) {
        result.eigenvectors[j].resize(n);
        for (int i = 0; i < n; ++i) {
            result.eigenvectors[j][i] = V[i * n + j];
        }
    }
    return result;
}

/** @brief 三对角化+QR @param matrix 矩阵 @param n 阶数 @return 结果 */
SymmetricEigen4::EigenResult SymmetricEigen4::tridiagQRMethod(
    const QVector<double>& matrix, int n)
{
    EigenResult result;
    QVector<double> diag, subdiag, transform;
    tridiagonalize(matrix, n, diag, subdiag, transform);

    implicitQLShift(diag, subdiag, transform, n);

    result.eigenvalues = diag;
    result.converged = true;
    result.iterations = m_maxIterations;

    /* 提取特征向量 */
    result.eigenvectors.resize(n);
    for (int j = 0; j < n; ++j) {
        result.eigenvectors[j].resize(n);
        for (int i = 0; i < n; ++i) {
            result.eigenvectors[j][i] = transform[i * n + j];
        }
    }
    return result;
}

/** @brief 三对角化(Householder) @param matrix 矩阵 @param n 阶数 @param diagonal 对角线输出 @param subdiagonal 次对角线输出 @param transform 变换矩阵输出 */
void SymmetricEigen4::tridiagonalize(const QVector<double>& matrix, int n,
                                      QVector<double>& diagonal,
                                      QVector<double>& subdiagonal,
                                      QVector<double>& transform)
{
    QVector<double> A = matrix;
    diagonal.resize(n);
    subdiagonal.resize(n);
    transform.resize(n * n);
    for (int i = 0; i < n * n; ++i) transform[i] = 0.0;
    for (int i = 0; i < n; ++i) transform[i * n + i] = 1.0;

    for (int k = 0; k < n - 2; ++k) {
        /* 计算Householder向量 */
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i) {
            sigma += A[i * n + k] * A[i * n + k];
        }
        double alpha = A[(k + 1) * n + k];
        double beta = (alpha >= 0 ? -1.0 : 1.0) * qSqrt(alpha * alpha + sigma);
        if (qFuzzyCompare(qAbs(beta), 0.0)) continue;

        double v1 = alpha - beta;
        double s = qSqrt(v1 * v1 + sigma);
        if (s < m_tolerance) continue;

        QVector<double> v(n, 0.0);
        v[k + 1] = v1 / s;
        for (int i = k + 2; i < n; ++i) {
            v[i] = A[i * n + k] / s;
        }

        /* A = (I - 2vv^T) A (I - 2vv^T) */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) dot += v[i] * A[i * n + j];
            for (int i = 0; i < n; ++i) A[i * n + j] -= 2.0 * v[i] * dot;
        }
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < n; ++j) dot += A[i * n + j] * v[j];
            for (int j = 0; j < n; ++j) A[i * n + j] -= 2.0 * dot * v[j];
        }

        /* 累积变换 */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < n; ++j) dot += transform[i * n + j] * v[j];
            for (int j = 0; j < n; ++j) transform[i * n + j] -= 2.0 * dot * v[j];
        }
    }

    for (int i = 0; i < n; ++i) diagonal[i] = A[i * n + i];
    for (int i = 1; i < n; ++i) subdiagonal[i] = A[i * n + i - 1];
    subdiagonal[0] = 0.0;
}

/** @brief 隐式QL位移迭代 @param diag 对角线 @param subdiag 次对角线 @param transform 变换矩阵 @param n 阶数 */
void SymmetricEigen4::implicitQLShift(QVector<double>& diag,
                                       QVector<double>& subdiag,
                                       QVector<double>& transform, int n)
{
    for (int l = 0; l < n; ++l) {
        int iter = 0;
        while (iter < m_maxIterations) {
            int m = l;
            while (m < n - 1) {
                double dd = qAbs(diag[m]) + qAbs(diag[m + 1]);
                if (qAbs(subdiag[m + 1]) + dd == dd) break;
                ++m;
            }
            if (m == l) break;

            double g = (diag[l + 1] - diag[l]) / (2.0 * subdiag[l + 1]);
            double r = qSqrt(g * g + 1.0);
            g = diag[m] - diag[l] + subdiag[l + 1]
                / (g + (g >= 0 ? qAbs(r) : -qAbs(r)));

            double s = 1.0, c = 1.0, p = 0.0;
            for (int i = m - 1; i >= l; --i) {
                double f = s * subdiag[i + 1];
                double b = c * subdiag[i + 1];
                r = qSqrt(f * f + g * g);
                subdiag[i + 1] = r;
                if (r < m_tolerance) {
                    diag[i + 1] -= p;
                    subdiag[m + 1] = 0.0;
                    break;
                }
                s = f / r; c = g / r;
                g = diag[i + 1] - p;
                r = (diag[i] - g) * s + 2.0 * c * b;
                p = s * r;
                diag[i + 1] = g + p;
                g = c * r - b;

                /* 累积特征向量变换 */
                for (int k = 0; k < n; ++k) {
                    double t = transform[k * n + i + 1];
                    transform[k * n + i + 1] = s * transform[k * n + i] + c * t;
                    transform[k * n + i] = c * transform[k * n + i] - s * t;
                }
                ++iter;
            }
            if (qAbs(subdiag[m + 1]) < m_tolerance) break;
        }
    }
}

/** @brief Sturm二分法 @param matrix 矩阵 @param n 阶数 @return 结果 */
SymmetricEigen4::EigenResult SymmetricEigen4::bisectionMethod(
    const QVector<double>& matrix, int n)
{
    EigenResult result;
    QVector<double> diag, subdiag, dummy;
    tridiagonalize(matrix, n, diag, subdiag, dummy);

    /* 确定特征值范围 */
    double lower = diag[0] - qAbs(subdiag[1]);
    double upper = diag[0] + qAbs(subdiag[1]);
    for (int i = 1; i < n; ++i) {
        double off = qAbs(subdiag[qMin(i + 1, n - 1)]);
        lower = qMin(lower, diag[i] - off);
        upper = qMax(upper, diag[i] + off);
    }
    double range = upper - lower;
    lower -= range * 0.01;
    upper += range * 0.01;

    /* 逐个二分求特征值 */
    result.eigenvalues.resize(n);
    for (int k = 0; k < n; ++k) {
        double lo = lower, hi = upper;
        for (int iter = 0; iter < 100; ++iter) {
            double mid = (lo + hi) / 2.0;
            int count = sturmCount(diag, subdiag, mid, n);
            if (count <= k) {
                lo = mid;
            } else {
                hi = mid;
            }
            if (hi - lo < m_tolerance) break;
        }
        result.eigenvalues[k] = (lo + hi) / 2.0;
    }

    result.converged = true;
    return result;
}

/** @brief 分治法 @param matrix 矩阵 @param n 阶数 @return 结果 */
SymmetricEigen4::EigenResult SymmetricEigen4::divideConquerMethod(
    const QVector<double>& matrix, int n)
{
    /* 分治: 对小规模用Jacobi, 大规模递归分裂 */
    if (n <= 16) {
        return jacobiMethod(matrix, n);
    }

    EigenResult result;
    int half = n / 2;

    /* 构造修正矩阵: 去掉次对角线连接 */
    QVector<double> top(half * half, 0.0);
    QVector<double> bot((n - half) * (n - half), 0.0);
    for (int i = 0; i < half; ++i) {
        for (int j = 0; j < half; ++j) {
            top[i * half + j] = matrix[i * n + j];
        }
    }
    for (int i = half; i < n; ++i) {
        for (int j = half; j < n; ++j) {
            bot[(i - half) * (n - half) + (j - half)] = matrix[i * n + j];
        }
    }

    EigenResult r1 = divideConquerMethod(top, half);
    EigenResult r2 = divideConquerMethod(bot, n - half);

    /* 合并特征值 */
    result.eigenvalues.reserve(n);
    result.eigenvalues = r1.eigenvalues + r2.eigenvalues;
    std::sort(result.eigenvalues.begin(), result.eigenvalues.end());

    /* 合并特征向量(嵌入到大空间) */
    result.eigenvectors.resize(n);
    for (int j = 0; j < half; ++j) {
        result.eigenvectors[j].resize(n, 0.0);
        for (int i = 0; i < half; ++i) {
            result.eigenvectors[j][i] = r1.eigenvectors[j][i];
        }
    }
    for (int j = 0; j < n - half; ++j) {
        result.eigenvectors[half + j].resize(n, 0.0);
        for (int i = 0; i < n - half; ++i) {
            result.eigenvectors[half + j][half + i] = r2.eigenvectors[j][i];
        }
    }

    result.converged = r1.converged && r2.converged;
    return result;
}

/** @brief Sturm计数: 小于x的特征值个数 @param diag 对角线 @param subdiag 次对角线 @param x 值 @param n 阶数 @return 计数 */
int SymmetricEigen4::sturmCount(const QVector<double>& diag,
                                 const QVector<double>& subdiag,
                                 double x, int n) const
{
    int count = 0;
    double d = 1.0;
    for (int i = 0; i < n; ++i) {
        if (qFuzzyCompare(d, 0.0)) d = m_tolerance;
        d = diag[i] - x - subdiag[qMax(0, i)] * subdiag[qMax(0, i)] / d;
        if (d < 0) ++count;
    }
    return count;
}

/** @brief 重置统计 */
void SymmetricEigen4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
