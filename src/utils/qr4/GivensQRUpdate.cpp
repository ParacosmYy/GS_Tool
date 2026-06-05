/**
 * @file GivensQRUpdate.cpp
 * @brief 增量QR分解更新实现
 */

#include "utils/qr4/GivensQRUpdate.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

GivensQRUpdate::GivensQRUpdate(QObject* parent)
    : QObject(parent)
{
}

QVector<double> GivensQRUpdate::initialize(const QVector<double>& matrix,
                                            int rows, int cols)
{
    QElapsedTimer timer;
    timer.start();

    /* 使用Givens旋转进行QR分解, 只保留R */
    QVector<double> R = matrix;
    int m = rows;
    int n = cols;

    for (int j = 0; j < n; ++j) {
        for (int i = m - 1; i > j; --i) {
            double a = R[(i - 1) * n + j];
            double b = R[i * n + j];
            if (qFuzzyIsNull(b)) continue;

            auto rotParams = givensRotation(a, b);
            double c = rotParams.first.first;
            double s = rotParams.first.second;

            /* 将旋转应用到第i-1行和第i行的所有列 */
            for (int k = j; k < n; ++k) {
                double r1 = R[(i - 1) * n + k];
                double r2 = R[i * n + k];
                R[(i - 1) * n + k] = c * r1 + s * r2;
                R[i * n + k] = -s * r1 + c * r2;
            }
        }
    }

    /* 提取上三角部分(n x n) */
    QVector<double> upperTri(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            upperTri[i * n + j] = R[i * n + j];
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions + m_stats.totalDeletions > 0)
        ? m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions) : 0.0;

    return upperTri;
}

QVector<double> GivensQRUpdate::insertRow(const QVector<double>& R,
                                           const QVector<double>& newRow)
{
    QElapsedTimer timer;
    timer.start();

    int n = newRow.size();
    QVector<double> updated = R;

    /* 将新行附加为R的最后一行: 扩展为(n+1) x n */
    QVector<double> augmented((n + 1) * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            augmented[i * n + j] = updated[i * n + j];
        }
    }
    for (int j = 0; j < n; ++j) {
        augmented[n * n + j] = newRow[j];
    }

    /* 用Givens旋转消去最后一行的非零元素, 恢复上三角 */
    for (int j = 0; j < n; ++j) {
        double diag = augmented[j * n + j];
        double elem = augmented[n * n + j];
        if (qFuzzyIsNull(elem)) continue;

        auto rotParams = givensRotation(diag, elem);
        double c = rotParams.first.first;
        double s = rotParams.first.second;

        /* 旋转第j行和第n行 */
        for (int k = j; k < n; ++k) {
            double r1 = augmented[j * n + k];
            double r2 = augmented[n * n + k];
            augmented[j * n + k] = c * r1 + s * r2;
            augmented[n * n + k] = -s * r1 + c * r2;
        }
    }

    /* 提取上三角(n x n) */
    QVector<double> result(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            result[i * n + j] = augmented[i * n + j];
        }
    }

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions);

    emit rowInserted(n);
    return result;
}

QVector<double> GivensQRUpdate::deleteRow(const QVector<double>& R,
                                           int rowIdx,
                                           const QVector<double>& removedRow)
{
    QElapsedTimer timer;
    timer.start();

    int n = removedRow.size();
    QVector<double> updated = R;

    /* 将被删除行的信息反映到R中 */
    /* 构造 [R; removedRow] 形式, 然后消去removedRow */
    QVector<double> augmented((n + 1) * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            augmented[i * n + j] = updated[i * n + j];
        }
    }
    for (int j = 0; j < n; ++j) {
        augmented[n * n + j] = removedRow[j];
    }

    /* 用Givens旋转消去removedRow */
    for (int j = 0; j < n; ++j) {
        double diag = augmented[j * n + j];
        double elem = augmented[n * n + j];
        if (qFuzzyIsNull(elem)) continue;

        auto rotParams = givensRotation(diag, elem);
        double c = rotParams.first.first;
        double s = rotParams.first.second;

        for (int k = j; k < n; ++k) {
            double r1 = augmented[j * n + k];
            double r2 = augmented[n * n + k];
            augmented[j * n + k] = c * r1 + s * r2;
            augmented[n * n + k] = -s * r1 + c * r2;
        }
    }

    /* 恢复上三角: 对R的各行重新用Givens旋转恢复 */
    for (int i = 1; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            double elem = augmented[i * n + j];
            if (qFuzzyIsNull(elem)) continue;

            double diag = augmented[j * n + j];
            auto rotParams = givensRotation(diag, elem);
            double c = rotParams.first.first;
            double s = rotParams.first.second;

            for (int k = j; k < n; ++k) {
                double r1 = augmented[j * n + k];
                double r2 = augmented[i * n + k];
                augmented[j * n + k] = c * r1 + s * r2;
                augmented[i * n + k] = -s * r1 + c * r2;
            }
            augmented[i * n + j] = 0.0;
        }
    }

    QVector<double> result(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            result[i * n + j] = augmented[i * n + j];
        }
    }

    m_stats.totalDeletions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions);

    emit rowDeleted(rowIdx);
    return result;
}

QVector<double> GivensQRUpdate::solveUpperTriangular(
    const QVector<double>& R,
    const QVector<double>& b,
    int n) const
{
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = b[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= R[i * n + j] * x[j];
        }
        double diag = R[i * n + i];
        x[i] = (qFuzzyIsNull(diag)) ? 0.0 : sum / diag;
    }
    return x;
}

QPair<QPair<double, double>, double> GivensQRUpdate::givensRotation(
    double a, double b)
{
    double r = qSqrt(a * a + b * b);
    if (qFuzzyIsNull(r)) {
        return {{1.0, 0.0}, 0.0};
    }
    double c = a / r;
    double s = b / r;
    return {{c, s}, r};
}

void GivensQRUpdate::applyGivens(QVector<double>& R, int n,
                                  double c, double s,
                                  int row1, int row2) const
{
    for (int k = 0; k < n; ++k) {
        double r1 = R[row1 * n + k];
        double r2 = R[row2 * n + k];
        R[row1 * n + k] = c * r1 + s * r2;
        R[row2 * n + k] = -s * r1 + c * r2;
    }
}

GivensQRUpdate::Stats GivensQRUpdate::stats() const
{
    return m_stats;
}

void GivensQRUpdate::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
