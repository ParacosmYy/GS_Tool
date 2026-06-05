/**
 * @file HouseholderBidiag.cpp
 * @brief Householder双对角化实现 — SVD预处理变换
 */

#include "utils/matrix11/HouseholderBidiag.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
HouseholderBidiag::HouseholderBidiag(QObject* parent)
    : QObject(parent)
{
}

/** @brief 执行双对角化 @param matrix 输入矩阵(m×n) @return 双对角化结果 */
HouseholderBidiag::BidiagResult HouseholderBidiag::bidiagonalize(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    BidiagResult result;
    int m = matrix.size();
    if (m == 0) return result;
    int n = matrix[0].size();
    if (n == 0) return result;

    m_lastDims = {m, n, qMin(m, n)};

    /* 拷贝工作矩阵 */
    result.B = matrix;

    /* 初始化U = I_m×m, V = I_n×n */
    result.U.resize(m);
    for (int i = 0; i < m; ++i) {
        result.U[i].resize(m, 0.0);
        result.U[i][i] = 1.0;
    }
    result.V.resize(n);
    for (int i = 0; i < n; ++i) {
        result.V[i].resize(n, 0.0);
        result.V[i][i] = 1.0;
    }

    int steps = qMin(m, n);
    result.householderSteps = 0;

    for (int k = 0; k < steps; ++k) {
        /* 列方向Householder: 消去B[k+1:m, k] */
        if (k < m - 1) {
            int colLen = m - k;
            QVector<double> col(colLen);
            for (int i = 0; i < colLen; ++i) {
                col[i] = result.B[k + i][k];
            }

            double norm = 0.0;
            for (double v : col) norm += v * v;
            norm = qSqrt(norm);

            double sigma = (col[0] >= 0) ? norm : -norm;
            auto [v, beta] = householderVector(col, sigma);

            applyLeftTransform(result.B, v, beta, k, k);
            accumulateTransform(result.U, v, beta, k);
            result.householderSteps++;
        }

        /* 行方向Householder: 消去B[k, k+2:n] */
        if (k < n - 2) {
            int rowLen = n - k - 1;
            QVector<double> row(rowLen);
            for (int j = 0; j < rowLen; ++j) {
                row[j] = result.B[k][k + 1 + j];
            }

            double norm = 0.0;
            for (double v : row) norm += v * v;
            norm = qSqrt(norm);

            double sigma = (row[0] >= 0) ? norm : -norm;
            auto [v, beta] = householderVector(row, sigma);

            applyRightTransform(result.B, v, beta, k, k + 1);
            accumulateTransform(result.V, v, beta, k + 1);
            result.householderSteps++;
        }
    }

    /* 更新统计 */
    m_stats.totalBidiagonalizations++;
    m_stats.totalElementsProcessed += m * n;
    m_stats.lastOrthogonalityError = verifyOrthogonality(result.U);
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalBidiagonalizations);

    emit bidiagonalizationComplete(result.householderSteps,
                                   m_stats.lastOrthogonalityError);
    return result;
}

/** @brief 仅计算双对角矩阵(不累积U和V) @param matrix 输入矩阵 @return 双对角矩阵 */
QVector<QVector<double>> HouseholderBidiag::bidiagonalizeFast(
    const QVector<QVector<double>>& matrix)
{
    int m = matrix.size();
    if (m == 0) return {};
    int n = matrix[0].size();
    if (n == 0) return {};

    QVector<QVector<double>> B = matrix;
    int steps = qMin(m, n);

    for (int k = 0; k < steps; ++k) {
        if (k < m - 1) {
            int colLen = m - k;
            QVector<double> col(colLen);
            for (int i = 0; i < colLen; ++i) col[i] = B[k + i][k];

            double norm = 0.0;
            for (double v : col) norm += v * v;
            norm = qSqrt(norm);
            double sigma = (col[0] >= 0) ? norm : -norm;
            auto [v, beta] = householderVector(col, sigma);
            applyLeftTransform(B, v, beta, k, k);
        }

        if (k < n - 2) {
            int rowLen = n - k - 1;
            QVector<double> row(rowLen);
            for (int j = 0; j < rowLen; ++j) row[j] = B[k][k + 1 + j];

            double norm = 0.0;
            for (double v : row) norm += v * v;
            norm = qSqrt(norm);
            double sigma = (row[0] >= 0) ? norm : -norm;
            auto [v, beta] = householderVector(row, sigma);
            applyRightTransform(B, v, beta, k, k + 1);
        }
    }

    m_stats.totalBidiagonalizations++;
    m_stats.totalElementsProcessed += m * n;
    return B;
}

/** @brief 验证正交性: ||U^T*U - I||_F @param U 矩阵 @return Frobenius范数误差 */
double HouseholderBidiag::verifyOrthogonality(const QVector<QVector<double>>& U) const
{
    int n = U.size();
    if (n == 0) return 0.0;

    auto UtU = matMul(transpose(U), U);
    double error = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double delta = UtU[i][j] - ((i == j) ? 1.0 : 0.0);
            error += delta * delta;
        }
    }
    return qSqrt(error);
}

/** @brief 矩阵乘法 C = A * B @param A 矩阵A @param B 矩阵B @return 乘积 */
QVector<QVector<double>> HouseholderBidiag::matMul(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    int m = A.size();
    if (m == 0) return {};
    int p = A[0].size();
    int n = (B.isEmpty()) ? 0 : B[0].size();
    if (p == 0 || n == 0 || B.size() != p) return {};

    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < p; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    return C;
}

/** @brief 矩阵转置 @param A 输入矩阵 @return 转置矩阵 */
QVector<QVector<double>> HouseholderBidiag::transpose(
    const QVector<QVector<double>>& A)
{
    if (A.isEmpty()) return {};
    int m = A.size();
    int n = A[0].size();
    QVector<QVector<double>> T(n, QVector<double>(m));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            T[j][i] = A[i][j];
        }
    }
    return T;
}

/** @brief 重置统计 */
void HouseholderBidiag::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 生成Householder向量 @param col 输入向量 @param sigma 期望符号 @return (v, beta) */
QPair<QVector<double>, double> HouseholderBidiag::householderVector(
    const QVector<double>& col, double sigma) const
{
    int n = col.size();
    QVector<double> v = col;

    double alpha = col[0];
    v[0] = alpha + sigma;

    double normV = 0.0;
    for (double x : v) normV += x * x;

    double beta = 0.0;
    if (normV > 1e-30) {
        beta = 2.0 / normV;
        double scale = 1.0 / qSqrt(normV);
        for (double& x : v) x *= scale;
        /* 重新计算beta: v已归一化 */
        beta = 2.0;
    }

    return {v, beta};
}

/** @brief 应用左侧Householder变换 */
void HouseholderBidiag::applyLeftTransform(QVector<QVector<double>>& matrix,
                                            const QVector<double>& v,
                                            double beta,
                                            int rowStart, int colStart)
{
    int m = matrix.size();
    int n = (m > 0) ? matrix[0].size() : 0;
    int vLen = v.size();

    /* p = beta * B[rowStart:, colStart:]^T * v */
    QVector<double> p(n - colStart, 0.0);
    for (int j = colStart; j < n; ++j) {
        for (int i = 0; i < vLen; ++i) {
            p[j - colStart] += v[i] * matrix[rowStart + i][j];
        }
        p[j - colStart] *= beta;
    }

    /* B -= v * p^T */
    for (int i = 0; i < vLen; ++i) {
        for (int j = colStart; j < n; ++j) {
            matrix[rowStart + i][j] -= v[i] * p[j - colStart];
        }
    }
}

/** @brief 应用右侧Householder变换 */
void HouseholderBidiag::applyRightTransform(QVector<QVector<double>>& matrix,
                                             const QVector<double>& v,
                                             double beta,
                                             int rowStart, int colStart)
{
    int m = matrix.size();
    int vLen = v.size();

    /* p = beta * B[rowStart:, colStart:] * v */
    QVector<double> p(m - rowStart, 0.0);
    for (int i = rowStart; i < m; ++i) {
        for (int j = 0; j < vLen; ++j) {
            p[i - rowStart] += matrix[i][colStart + j] * v[j];
        }
        p[i - rowStart] *= beta;
    }

    /* B -= p * v^T */
    for (int i = rowStart; i < m; ++i) {
        for (int j = 0; j < vLen; ++j) {
            matrix[i][colStart + j] -= p[i - rowStart] * v[j];
        }
    }
}

/** @brief 累积变换到U或V */
void HouseholderBidiag::accumulateTransform(QVector<QVector<double>>& mat,
                                             const QVector<double>& v,
                                             double beta,
                                             int startIdx)
{
    int n = mat.size();
    int vLen = v.size();

    /* p = beta * mat[startIdx:, startIdx:] * v */
    QVector<double> p(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < vLen; ++j) {
            p[i] += mat[i][startIdx + j] * v[j];
        }
        p[i] *= beta;
    }

    /* mat -= p * v^T */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < vLen; ++j) {
            mat[i][startIdx + j] -= p[i] * v[j];
        }
    }
}
