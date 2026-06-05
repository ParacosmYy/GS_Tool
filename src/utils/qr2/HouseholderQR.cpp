/**
 * @file HouseholderQR.cpp
 * @brief QR分解实现 — Householder反射
 */

#include "utils/qr2/HouseholderQR.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
HouseholderQR::HouseholderQR(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief QR分解 */
HouseholderQR::QrResult HouseholderQR::decompose(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    QrResult result;
    int m = matrix.size();
    if (m == 0) return result;
    int n = matrix[0].size();
    if (n == 0 || m < n) return result;

    /* 拷贝到R */
    result.R = matrix;
    result.Q = QVector<QVector<double>>(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) result.Q[i][i] = 1.0;

    for (int col = 0; col < n; ++col) {
        /* 计算Householder向量 */
        double norm = 0.0;
        for (int i = col; i < m; ++i)
            norm += result.R[i][col] * result.R[i][col];
        norm = qSqrt(norm);

        if (norm < 1e-15) continue;

        double sign = (result.R[col][col] >= 0) ? 1.0 : -1.0;
        double v0 = result.R[col][col] + sign * norm;

        /* 归一化Householder向量 */
        QVector<double> v(m, 0.0);
        v[col] = v0;
        for (int i = col + 1; i < m; ++i)
            v[i] = result.R[i][col];

        double vNorm = 0.0;
        for (int i = col; i < m; ++i) vNorm += v[i] * v[i];
        if (vNorm < 1e-30) continue;

        /* H = I - 2vv^T/v^Tv */
        /* R = H * R */
        for (int j = col; j < n; ++j) {
            double dot = 0.0;
            for (int i = col; i < m; ++i)
                dot += v[i] * result.R[i][j];
            dot *= 2.0 / vNorm;
            for (int i = col; i < m; ++i)
                result.R[i][j] -= dot * v[i];
        }

        /* Q = Q * H */
        for (int j = 0; j < m; ++j) {
            double dot = 0.0;
            for (int i = col; i < m; ++i)
                dot += result.Q[j][i] * v[i];
            dot *= 2.0 / vNorm;
            for (int i = col; i < m; ++i)
                result.Q[j][i] -= dot * v[i];
        }
    }

    result.success = true;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalDecompositions;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecompositions);

    emit decompositionCompleted(m, n);
    return result;
}

/** @brief 最小二乘求解 */
QVector<double> HouseholderQR::solveLeastSquares(
    const QVector<QVector<double>>& A,
    const QVector<double>& b)
{
    int m = A.size();
    int n = (m > 0) ? A[0].size() : 0;
    if (m == 0 || n == 0) return QVector<double>();

    auto qr = decompose(A);
    if (!qr.success) return QVector<double>();

    /* Q^T * b */
    QVector<double> qtb(m, 0.0);
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            qtb[i] += qr.Q[i][j] * b[j]; // Q是正交的，Q^T = Q转置

    /* 注意: Q存储为行向量，实际是Q^T*Q = I的形式
       修正: Q^T[i][j] = Q[j][i] */
    qtb.fill(0.0);
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            qtb[i] += qr.Q[j][i] * b[j];

    /* 回代R*x = Q^T*b的前n行 */
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = qtb[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= qr.R[i][j] * x[j];
        if (qAbs(qr.R[i][i]) > 1e-15)
            x[i] /= qr.R[i][i];
    }

    return x;
}

/** @brief 重置统计 */
void HouseholderQR::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
