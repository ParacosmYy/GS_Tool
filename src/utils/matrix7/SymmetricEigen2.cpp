/**
 * @file SymmetricEigen2.cpp
 * @brief 对称矩阵特征值分解实现 — 三对角QL算法
 */

#include "utils/matrix7/SymmetricEigen2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

SymmetricEigen2::SymmetricEigen2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

SymmetricEigen2::EigenResult SymmetricEigen2::decompose(
    const QVector<double>& matrix, int n, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    EigenResult result;
    if (n <= 0 || matrix.size() < n * n) return result;

    /* 检查对称性(宽松容差) */
    bool sym = isSymmetric(matrix, n, 1e-6);

    /* Householder约化到三对角 */
    QVector<double> diagonal(n), subdiagonal(n);
    QVector<double> transform(n * n, 0.0);

    /* 初始化变换矩阵为单位阵 */
    for (int i = 0; i < n; ++i) transform[i * n + i] = 1.0;

    householderTridiagonal(matrix, n, diagonal, subdiagonal, &transform);

    /* QL迭代求解 */
    result.iterations = qlIterate(diagonal, subdiagonal, transform, n,
                                   maxIterations);
    result.converged = (result.iterations >= 0);

    if (result.iterations < 0) result.iterations = maxIterations;

    /* 排序特征值(升序)并重排特征向量 */
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(),
        [&diagonal](int a, int b) { return diagonal[a] < diagonal[b]; });

    result.eigenvalues.resize(n);
    result.eigenvectors.resize(n);
    for (int i = 0; i < n; ++i) {
        result.eigenvalues[i] = diagonal[indices[i]];
        result.eigenvectors[i].resize(n);
        for (int j = 0; j < n; ++j) {
            result.eigenvectors[i][j] = transform[j * n + indices[i]];
        }
    }

    /* 更新统计 */
    ++m_stats.totalDecompositions;
    m_stats.totalIterations += qAbs(result.iterations);
    if (result.converged) ++m_stats.totalConverged;
    else ++m_stats.totalDiverged;
    m_stats.maxMatrixSize = qMax(m_stats.maxMatrixSize, n);

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decomposed(n, result.converged);
    return result;
}

SymmetricEigen2::EigenResult SymmetricEigen2::decomposeTridiagonal(
    const QVector<double>& diagonal, const QVector<double>& subdiagonal,
    int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    EigenResult result;
    int n = diagonal.size();
    if (n <= 0) return result;

    QVector<double> d = diagonal;
    QVector<double> e = subdiagonal;

    /* 构造单位变换矩阵 */
    QVector<double> z(n * n, 0.0);
    for (int i = 0; i < n; ++i) z[i * n + i] = 1.0;

    result.iterations = qlIterate(d, e, z, n, maxIterations);
    result.converged = (result.iterations >= 0);
    if (result.iterations < 0) result.iterations = maxIterations;

    /* 排序 */
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(),
        [&d](int a, int b) { return d[a] < d[b]; });

    result.eigenvalues.resize(n);
    result.eigenvectors.resize(n);
    for (int i = 0; i < n; ++i) {
        result.eigenvalues[i] = d[indices[i]];
        result.eigenvectors[i].resize(n);
        for (int j = 0; j < n; ++j) {
            result.eigenvectors[i][j] = z[j * n + indices[i]];
        }
    }

    ++m_stats.totalDecompositions;
    if (result.converged) ++m_stats.totalConverged;
    else ++m_stats.totalDiverged;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decomposed(n, result.converged);
    return result;
}

QVector<double> SymmetricEigen2::eigenvaluesOnly(
    const QVector<double>& matrix, int n)
{
    if (n <= 0 || matrix.size() < n * n) return {};

    QVector<double> d(n), e(n);
    householderTridiagonal(matrix, n, d, e, nullptr);

    /* QL迭代(不累积变换) */
    QVector<double> z;  /* 空 = 不计算特征向量 */
    int result = qlIterate(d, e, z, n, 100);

    if (result < 0) return {};

    std::sort(d.begin(), d.end());
    return d;
}

void SymmetricEigen2::householderTridiagonal(
    const QVector<double>& matrix, int n,
    QVector<double>& diagonal, QVector<double>& subdiagonal,
    QVector<double>* transform)
{
    /* 复制矩阵到工作数组 */
    QVector<double> a = matrix;

    for (int k = 0; k < n - 2; ++k) {
        /* 计算Householder向量 */
        double scale = 0.0;
        for (int i = k + 2; i < n; ++i) {
            scale += qAbs(a[i * n + k]);
        }

        if (scale < 1e-30) {
            subdiagonal[k + 1] = a[(k + 1) * n + k];
            continue;
        }

        double h = 0.0;
        for (int i = k + 1; i < n; ++i) {
            double val = a[i * n + k];
            a[i * n + k] = val / scale;
            h += a[i * n + k] * val;
        }

        double f = a[(k + 1) * n + k];
        double g = (f >= 0) ? -qSqrt(h) : qSqrt(h);
        subdiagonal[k + 1] = scale * g;
        h -= f * g;
        a[(k + 1) * n + k] = f - g;

        /* 应用Householder变换 */
        QVector<double> p(n - k - 1, 0.0);
        for (int j = k + 1; j < n; ++j) {
            for (int i = k + 1; i < n; ++i) {
                p[j - k - 1] += a[j * n + i] * a[i * n + k];
            }
            p[j - k - 1] /= h;
        }

        double hh = 0.0;
        for (int i = k + 1; i < n; ++i) {
            hh += a[i * n + k] * p[i - k - 1];
        }
        hh /= (2.0 * h);

        for (int j = k + 1; j < n; ++j) {
            double pj = p[j - k - 1] - hh * a[j * n + k];
            for (int i = j; i < n; ++i) {
                a[i * n + j] -= a[i * n + k] * pj + a[j * n + k] * p[i - k - 1];
            }
        }

        /* 累积变换 */
        if (transform) {
            for (int i = 0; i < n; ++i) {
                double sum = 0.0;
                for (int j = k + 1; j < n; ++j) {
                    sum += (*transform)[i * n + j] * a[j * n + k];
                }
                for (int j = k + 1; j < n; ++j) {
                    (*transform)[i * n + j] -= sum * a[j * n + k] / h;
                }
            }
        }
    }

    if (n >= 2) subdiagonal[0] = 0.0;
    if (n >= 1) subdiagonal[n - 1] = 0.0;

    /* 提取对角线 */
    for (int i = 0; i < n; ++i) {
        diagonal[i] = a[i * n + i];
    }
}

bool SymmetricEigen2::isSymmetric(const QVector<double>& matrix, int n,
                                   double tolerance) const
{
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double diff = qAbs(matrix[i * n + j] - matrix[j * n + i]);
            double scale = qMax(qAbs(matrix[i * n + j]),
                                qAbs(matrix[j * n + i]));
            if (diff > tolerance * qMax(scale, 1.0)) return false;
        }
    }
    return true;
}

QVector<double> SymmetricEigen2::matMul(const QVector<double>& a,
                                         const QVector<double>& b, int n)
{
    QVector<double> c(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < n; ++k) {
            double aik = a[i * n + k];
            for (int j = 0; j < n; ++j) {
                c[i * n + j] += aik * b[k * n + j];
            }
        }
    }
    return c;
}

SymmetricEigen2::Stats SymmetricEigen2::stats() const
{
    return m_stats;
}

void SymmetricEigen2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

int SymmetricEigen2::qlIterate(QVector<double>& d, QVector<double>& e,
                                QVector<double>& z, int n, int maxIter)
{
    int totalIter = 0;

    /* 修正次对角线索引 */
    for (int i = 1; i < n; ++i) e[i - 1] = e[i];
    e[n - 1] = 0.0;

    for (int l = 0; l < n; ++l) {
        int iterCount = 0;
        int m = l;

        /* 寻找小次对角线元素 */
        while (m < n - 1) {
            double dd = qAbs(d[m]) + qAbs(d[m + 1]);
            if (qAbs(e[m]) + dd == dd) break;
            m++;
        }

        if (m != l) {
            if (++iterCount > maxIter) return -1;
            totalIter++;

            /* 隐式位移 */
            double g = (d[l + 1] - d[l]) / (2.0 * e[l]);
            double r = qSqrt(g * g + 1.0);
            g = d[m] - d[l] + e[l] / (g + ((g >= 0) ? qAbs(r) : -qAbs(r)));

            double s = 1.0, c = 1.0, p = 0.0;

            for (int i = m - 1; i >= l; --i) {
                double f = s * e[i];
                double b = c * e[i];

                if (qAbs(f) >= qAbs(g)) {
                    c = g / f;
                    r = qSqrt(c * c + 1.0);
                    e[i + 1] = f * r;
                    s = 1.0 / r;
                    c *= s;
                } else {
                    s = f / g;
                    r = qSqrt(s * s + 1.0);
                    e[i + 1] = g * r;
                    c = 1.0 / r;
                    s *= c;
                }

                g = d[i + 1] - p;
                r = (d[i] - g) * s + 2.0 * c * b;
                p = s * r;
                d[i + 1] = g + p;
                g = c * r - b;

                /* 累积特征向量变换 */
                if (!z.isEmpty()) {
                    for (int k = 0; k < n; ++k) {
                        double t = z[k * n + i + 1];
                        z[k * n + i + 1] = s * z[k * n + i] + c * t;
                        z[k * n + i] = c * z[k * n + i] - s * t;
                    }
                }
            }

            d[l] -= p;
            e[l] = g;
            e[m] = 0.0;

            /* 检查是否需要继续迭代 */
            l--;
        }
    }

    return totalIter;
}
