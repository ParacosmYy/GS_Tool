/**
 * @file LUDecomposition.cpp
 * @brief LUDecomposition 实现
 *
 * 实现LU分解：部分主元选取、前推/回代、行列式与逆矩阵计算。
 */

#include "utils/matrix171/LUDecomposition.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

LUDecomposition::LUDecomposition(QObject* parent)
    : QObject(parent)
{
}

LUDecomposition::~LUDecomposition() = default;

LUDecomposition::Result LUDecomposition::decompose(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    Result result;
    result.determinant = 1.0;
    result.sign = 1;

    if (n == 0) return result;

    /* Initialize L as identity, U as copy, P as identity */
    result.L.assign(n, QVector<double>(n, 0.0));
    result.U = matrix;
    result.P.resize(n);

    for (int i = 0; i < n; ++i) {
        result.L[i][i] = 1.0;
        result.P[i] = i;
    }

    for (int k = 0; k < n; ++k) {
        /* Find pivot: maximum element in column k below diagonal */
        double maxVal = qAbs(result.U[k][k]);
        int maxRow = k;
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(result.U[i][k]) > maxVal) {
                maxVal = qAbs(result.U[i][k]);
                maxRow = i;
            }
        }

        /* Swap rows if needed */
        if (maxRow != k) {
            std::swap(result.U[k], result.U[maxRow]);
            std::swap(result.L[k], result.L[maxRow]);
            std::swap(result.P[k], result.P[maxRow]);
            result.sign *= -1;
        }

        /* Check for singularity */
        if (qAbs(result.U[k][k]) < 1e-15) {
            result.determinant = 0.0;
            break;
        }

        /* Eliminate below pivot */
        for (int i = k + 1; i < n; ++i) {
            result.L[i][k] = result.U[i][k] / result.U[k][k];
            for (int j = k; j < n; ++j)
                result.U[i][j] -= result.L[i][k] * result.U[k][j];
        }
    }

    /* Compute determinant: product of U diagonal * sign */
    result.determinant = static_cast<double>(result.sign);
    for (int i = 0; i < n; ++i)
        result.determinant *= result.U[i][i];

    m_stats.totalDecompositions++;
    m_stats.lastMatrixSize = n;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalDecompositions + m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decompositionCompleted(n);
    return result;
}

QVector<double> LUDecomposition::forwardSub(const Result& lu,
                                             const QVector<double>& pb) const
{
    int n = pb.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        y[i] = pb[i];
        for (int j = 0; j < i; ++j)
            y[i] -= lu.L[i][j] * y[j];
    }
    return y;
}

QVector<double> LUDecomposition::backSub(const Result& lu,
                                           const QVector<double>& y) const
{
    int n = y.size();
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= lu.U[i][j] * x[j];
        if (qAbs(lu.U[i][i]) > 1e-15)
            x[i] /= lu.U[i][i];
    }
    return x;
}

QVector<double> LUDecomposition::solve(const Result& result,
                                        const QVector<double>& b) const
{
    int n = b.size();
    if (n == 0 || result.P.size() != n) return QVector<double>();

    /* Apply permutation: Pb */
    QVector<double> pb(n);
    for (int i = 0; i < n; ++i)
        pb[i] = b[result.P[i]];

    QVector<double> y = forwardSub(result, pb);
    QVector<double> x = backSub(result, y);
    return x;
}

QVector<QVector<double>> LUDecomposition::inverse(const Result& result, int n) const
{
    if (n == 0) return QVector<QVector<double>>();

    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    for (int col = 0; col < n; ++col) {
        QVector<double> e(n, 0.0);
        e[col] = 1.0;
        QVector<double> x = solve(result, e);
        for (int row = 0; row < n; ++row)
            inv[row][col] = (row < x.size()) ? x[row] : 0.0;
    }
    return inv;
}

double LUDecomposition::determinant(const QVector<QVector<double>>& matrix)
{
    Result r = decompose(matrix);
    return r.determinant;
}

void LUDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
