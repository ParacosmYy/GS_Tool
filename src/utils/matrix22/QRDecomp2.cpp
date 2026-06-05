/**
 * @file QRDecomp2.cpp
 * @brief 列主元QR分解实现 — Householder反射/秩揭示/最小二乘
 */

#include "utils/matrix22/QRDecomp2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
QRDecomp2::QRDecomp2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 带列主元的QR分解 @param matrix 输入矩阵 @param tolerance 容差 @return QR结果 */
QRDecomp2::QRResult QRDecomp2::decompose(
    const QVector<QVector<double>>& matrix, double tolerance)
{
    QElapsedTimer timer;
    timer.start();

    QRResult result;
    int m = matrix.size();
    if (m == 0) return result;
    int n = matrix[0].size();
    if (n == 0) return result;

    /* 初始化列置换向量和列范数 */
    result.permutation.resize(n);
    QVector<double> colNorms(n);
    for (int j = 0; j < n; ++j) {
        result.permutation[j] = j;
        colNorms[j] = 0.0;
        for (int i = 0; i < m; ++i) {
            colNorms[j] += matrix[i][j] * matrix[i][j];
        }
    }

    /* 工作矩阵: 深拷贝输入 */
    QVector<QVector<double>> work = matrix;

    /* 初始化Q为单位矩阵 */
    result.Q.resize(m);
    for (int i = 0; i < m; ++i) {
        result.Q[i].resize(m, 0.0);
        result.Q[i][i] = 1.0;
    }

    int rank = 0;
    int minDim = qMin(m, n);

    for (int k = 0; k < minDim; ++k) {
        /* 列主元选择: 选剩余列中范数最大的 */
        int pivotCol = k;
        double maxNorm = colNorms[result.permutation[k]];
        for (int j = k + 1; j < n; ++j) {
            if (colNorms[result.permutation[j]] > maxNorm) {
                maxNorm = colNorms[result.permutation[j]];
                pivotCol = j;
            }
        }

        /* 交换列 */
        if (pivotCol != k) {
            std::swap(result.permutation[k], result.permutation[pivotCol]);
        }

        /* 检查秩: 如果最大列范数接近零则停止 */
        if (maxNorm < tolerance) break;

        /* 提取第k列的Householder向量 */
        QVector<double> x(m - k);
        for (int i = k; i < m; ++i) {
            x[i - k] = work[i][result.permutation[k]];
        }

        auto refl = householderReflection(x);
        QVector<double>& v = refl.first;
        double beta = refl.second;

        /* 应用Householder反射到工作矩阵 */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < m - k; ++i) {
                dot += v[i] * work[k + i][result.permutation[j]];
            }
            for (int i = 0; i < m - k; ++i) {
                work[k + i][result.permutation[j]] -= beta * v[i] * dot;
            }
        }

        /* 累积Q矩阵 */
        for (int i = 0; i < m; ++i) {
            double dot = 0.0;
            for (int j = 0; j < m - k; ++j) {
                dot += result.Q[i][k + j] * v[j];
            }
            for (int j = 0; j < m - k; ++j) {
                result.Q[i][k + j] -= beta * v[j] * dot;
            }
        }

        /* 更新列范数 */
        for (int j = k + 1; j < n; ++j) {
            colNorms[result.permutation[j]] -=
                work[k][result.permutation[j]] * work[k][result.permutation[j]];
            colNorms[result.permutation[j]] = qMax(0.0, colNorms[result.permutation[j]]);
        }

        ++rank;
    }

    /* 提取R矩阵 */
    result.R.resize(m);
    for (int i = 0; i < m; ++i) {
        result.R[i].resize(n, 0.0);
        for (int j = 0; j < n; ++j) {
            result.R[i][j] = work[i][result.permutation[j]];
        }
    }
    result.rank = rank;

    /* 估计条件数 */
    result.conditionNumber = estimateConditionNumber(result.R);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalDecompositions;
    m_stats.totalElementsProcessed += m * n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecompositions);
    if (result.conditionNumber > m_stats.worstConditionNumber) {
        m_stats.worstConditionNumber = result.conditionNumber;
    }

    emit decompositionComplete(m, n, rank);
    return result;
}

/** @brief Householder反射 @param x 输入向量 @return (反射向量, 系数beta) */
QPair<QVector<double>, double> QRDecomp2::householderReflection(
    const QVector<double>& x)
{
    int n = x.size();
    if (n == 0) return {{}, 0.0};

    QVector<double> v = x;
    double sigma = 0.0;
    for (int i = 1; i < n; ++i) {
        sigma += x[i] * x[i];
    }

    double alpha = qSqrt(x[0] * x[0] + sigma);
    if (x[0] >= 0) alpha = -alpha;

    double beta = 0.0;
    if (sigma < 1e-20 && qAbs(x[0]) < 1e-20) {
        /* 零向量, 不需要反射 */
        v.fill(0.0);
        beta = 0.0;
    } else {
        v[0] = x[0] - alpha;
        double vNormSq = v[0] * v[0] + sigma;
        beta = (vNormSq > 1e-20) ? 2.0 / vNormSq : 0.0;
    }

    return {v, beta};
}

/** @brief 列主元选择 @param matrix 矩阵 @param col 起始列 @param pivotCandidates 候选列 */
int QRDecomp2::selectPivotColumn(const QVector<QVector<double>>& matrix,
                                 int col, const QVector<int>& pivotCandidates)
{
    if (matrix.isEmpty() || pivotCandidates.isEmpty()) return 0;

    int bestCol = pivotCandidates[0];
    double bestNorm = 0.0;
    int rows = matrix.size();

    for (int c : pivotCandidates) {
        double norm = 0.0;
        for (int i = col; i < rows; ++i) {
            norm += matrix[i][c] * matrix[i][c];
        }
        if (norm > bestNorm) {
            bestNorm = norm;
            bestCol = c;
        }
    }
    return bestCol;
}

/** @brief 最小二乘求解 @param A 系数矩阵 @param b 右侧向量 @return 解向量 */
QVector<double> QRDecomp2::leastSquaresSolve(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size();
    if (m == 0) return {};
    int n = A[0].size();
    if (b.size() != m) return {};

    /* QR分解 */
    QRResult qr = decompose(A);
    if (qr.rank == 0) return {};

    /* Q^T * b */
    QVector<double> qtb(m, 0.0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            qtb[i] += qr.Q[j][i] * b[j];
        }
    }

    /* 回代 R*x = Q^T*b(前rank行) */
    QVector<double> x(n, 0.0);
    for (int i = qr.rank - 1; i >= 0; --i) {
        double sum = qtb[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= qr.R[i][j] * x[j];
        }
        x[i] = (qAbs(qr.R[i][i]) > 1e-15) ? sum / qr.R[i][i] : 0.0;
    }

    /* 还原列置换 */
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[qr.permutation[i]] = x[i];
    }

    /* 计算残差 */
    double residual = 0.0;
    QVector<double> ax = matVecMultiply(A, result);
    for (int i = 0; i < m; ++i) {
        double r = b[i] - ax[i];
        residual += r * r;
    }
    residual = qSqrt(residual);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalSolves;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecompositions + m_stats.totalSolves);

    emit solveComplete(residual);
    return result;
}

/** @brief 估计条件数 @param R 上三角矩阵 @return 条件数 */
double QRDecomp2::estimateConditionNumber(
    const QVector<QVector<double>>& R) const
{
    if (R.isEmpty()) return 0.0;
    int n = R[0].size();

    /* 对角线最大/最小绝对值比 */
    double maxDiag = 0.0, minDiag = 1e18;
    int minDim = qMin(R.size(), n);
    for (int i = 0; i < minDim; ++i) {
        double d = qAbs(R[i][i]);
        if (d > maxDiag) maxDiag = d;
        if (d < minDiag && d > 1e-15) minDiag = d;
    }

    return (minDiag > 1e-15) ? maxDiag / minDiag : 1e18;
}

/** @brief 重置统计 */
void QRDecomp2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 向量范数 @param v 向量 @return L2范数 */
double QRDecomp2::vectorNorm(const QVector<double>& v) const
{
    double sum = 0.0;
    for (double x : v) sum += x * x;
    return qSqrt(sum);
}

/** @brief 矩阵向量乘法 @param M 矩阵 @param v 向量 @return 结果 */
QVector<double> QRDecomp2::matVecMultiply(
    const QVector<QVector<double>>& M, const QVector<double>& v) const
{
    int m = M.size();
    if (m == 0) return {};
    int n = M[0].size();
    QVector<double> result(m, 0.0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            result[i] += M[i][j] * v[j];
        }
    }
    return result;
}

/** @brief 应用Householder反射到子矩阵 */
void QRDecomp2::applyHouseholder(QVector<QVector<double>>& matrix,
                                 const QVector<double>& v, double beta,
                                 int startRow, int startCol)
{
    int m = matrix.size();
    int n = matrix[0].size();
    int vLen = v.size();

    for (int j = startCol; j < n; ++j) {
        double dot = 0.0;
        for (int i = 0; i < vLen && startRow + i < m; ++i) {
            dot += v[i] * matrix[startRow + i][j];
        }
        for (int i = 0; i < vLen && startRow + i < m; ++i) {
            matrix[startRow + i][j] -= beta * v[i] * dot;
        }
    }
}
