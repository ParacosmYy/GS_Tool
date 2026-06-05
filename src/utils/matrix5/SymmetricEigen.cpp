/**
 * @file SymmetricEigen.cpp
 * @brief 对称矩阵特征值分解实现
 */

#include "utils/matrix5/SymmetricEigen.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

SymmetricEigen::SymmetricEigen(QObject* parent)
    : QObject(parent)
{
}

void SymmetricEigen::setMaxIterations(int maxIterations)
{
    m_maxIterations = qMax(1, maxIterations);
}

void SymmetricEigen::setTolerance(double tolerance)
{
    m_tolerance = qMax(1e-15, tolerance);
}

SymmetricEigen::EigenResult SymmetricEigen::decompose(
    const QVector<double>& matrix, int n) const
{
    QElapsedTimer timer;
    timer.start();

    EigenResult result;

    if (n <= 0 || matrix.size() < n * n) {
        m_timeSum += timer.elapsed();
        return result;
    }

    int maxIter = (m_maxIterations > 0) ? m_maxIterations : 100 * n;

    /* 复制矩阵到工作数组A */
    QVector<double> A = matrix;

    /* 初始化特征向量矩阵为单位阵 */
    QVector<QVector<double>> Q(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) Q[i][i] = 1.0;

    int totalRotations = 0;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 找最大非对角元素 */
        auto pivot = findMaxOffDiag(A, n);
        int p = pivot.first;
        int q = pivot.second;

        /* 检查收敛 */
        if (qFuzzyIsNull(A[p * n + q]) ||
            qAbs(A[p * n + q]) < m_tolerance) {
            result.finalOffDiag = offDiagNorm(A, n);
            break;
        }

        /* 计算旋转角度 */
        double app = A[p * n + p];
        double aqq = A[q * n + q];
        double apq = A[p * n + q];

        double theta;
        if (qFuzzyCompare(app, aqq)) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * qAtan2(2.0 * apq, app - aqq);
        }

        double c = qCos(theta);
        double s = qSin(theta);

        /* 应用Jacobi旋转: A' = J^T * A * J */
        QVector<double> rowP(n), rowQ(n);
        for (int j = 0; j < n; ++j) {
            rowP[j] = A[p * n + j];
            rowQ[j] = A[q * n + j];
        }

        for (int j = 0; j < n; ++j) {
            A[p * n + j] = c * rowP[j] + s * rowQ[j];
            A[j * n + p] = A[p * n + j];
            A[q * n + j] = -s * rowP[j] + c * rowQ[j];
            A[j * n + q] = A[q * n + j];
        }

        A[p * n + p] = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        A[q * n + q] = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        A[p * n + q] = 0.0;
        A[q * n + p] = 0.0;

        /* 更新特征向量: Q' = Q * J */
        for (int i = 0; i < n; ++i) {
            double qip = Q[i][p];
            double qiq = Q[i][q];
            Q[i][p] = c * qip + s * qiq;
            Q[i][q] = -s * qip + c * qiq;
        }

        ++totalRotations;
        result.iterations = iter + 1;
    }

    /* 提取特征值 */
    result.eigenvalues.resize(n);
    for (int i = 0; i < n; ++i) {
        result.eigenvalues[i] = A[i * n + i];
    }

    /* 按特征值降序排列 */
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(),
              [this](int a, int b) {
                  return result.eigenvalues[a] > result.eigenvalues[b];
              });

    QVector<double> sortedVals(n);
    QVector<QVector<double>> sortedVecs(n, QVector<double>(n));
    for (int i = 0; i < n; ++i) {
        sortedVals[i] = result.eigenvalues[indices[i]];
        for (int j = 0; j < n; ++j) {
            sortedVecs[i][j] = Q[j][indices[i]];
        }
    }
    result.eigenvalues = sortedVals;
    result.eigenvectors = sortedVecs;
    result.finalOffDiag = offDiagNorm(A, n);

    m_stats.totalDecompositions++;
    m_stats.totalRotations += totalRotations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n, result.iterations);
    return result;
}

QVector<double> SymmetricEigen::eigenvaluesOnly(
    const QVector<double>& matrix, int n) const
{
    auto result = decompose(matrix, n);
    return result.eigenvalues;
}

QVector<double> SymmetricEigen::reconstruct(
    const EigenResult& result, int n) const
{
    /* A = Q * D * Q^T */
    QVector<double> A(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                /* Q[i][k] * D[k][k] * Q[j][k] */
                sum += result.eigenvectors[k][i] *
                       result.eigenvalues[k] *
                       result.eigenvectors[k][j];
            }
            A[i * n + j] = sum;
        }
    }
    return A;
}

double SymmetricEigen::conditionNumber(
    const QVector<double>& eigenvalues) const
{
    if (eigenvalues.isEmpty()) return 0.0;
    double maxEv = eigenvalues[0]; /* 假设已排序 */
    double minEv = eigenvalues.last();
    /* 取绝对值处理负特征值 */
    maxEv = qAbs(maxEv);
    for (double ev : eigenvalues) {
        double absEv = qAbs(ev);
        if (absEv > maxEv) maxEv = absEv;
    }
    for (double ev : eigenvalues) {
        double absEv = qAbs(ev);
        if (absEv > 0 && absEv < minEv) minEv = absEv;
    }
    return (minEv > 1e-15) ? maxEv / minEv : 1e18;
}

QPair<int, int> SymmetricEigen::findMaxOffDiag(
    const QVector<double>& A, int n) const
{
    int p = 0, q = 1;
    double maxVal = qAbs(A[0 * n + 1]);

    for (int i = 0; i < n - 1; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double val = qAbs(A[i * n + j]);
            if (val > maxVal) {
                maxVal = val;
                p = i;
                q = j;
            }
        }
    }
    return {p, q};
}

double SymmetricEigen::offDiagNorm(const QVector<double>& A, int n) const
{
    double sum = 0.0;
    for (int i = 0; i < n - 1; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double val = A[i * n + j];
            sum += val * val;
        }
    }
    return qSqrt(2.0 * sum);
}

SymmetricEigen::Stats SymmetricEigen::stats() const
{
    return m_stats;
}

void SymmetricEigen::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
