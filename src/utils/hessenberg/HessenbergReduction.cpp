/**
 * @file HessenbergReduction.cpp
 * @brief 上Hessenberg化简实现
 */

#include "utils/hessenberg/HessenbergReduction.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
HessenbergReduction::HessenbergReduction(QObject* parent)
    : QObject(parent)
{
}

/** @brief 化矩阵为上Hessenberg形式 */
QPair<QVector<QVector<double>>, QVector<QVector<double>>>
HessenbergReduction::reduce(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0) return {{}, {}};

    QVector<QVector<double>> H(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> Q(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        Q[i][i] = 1.0;
        for (int j = 0; j < n; ++j)
            H[i][j] = A[i][j];
    }

    for (int k = 0; k < n - 2; ++k) {
        /* 计算Householder向量 */
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i)
            norm += H[i][k] * H[i][k];
        norm = std::sqrt(norm);

        if (norm < 1e-15) continue;

        double s = (H[k + 1][k] >= 0.0) ? norm : -norm;
        double alpha = std::sqrt(2.0) / std::sqrt(norm * norm + std::abs(H[k + 1][k] * s));

        QVector<double> v(n, 0.0);
        v[k + 1] = (H[k + 1][k] + s) * alpha;
        for (int i = k + 2; i < n; ++i)
            v[i] = H[i][k] * alpha;

        /* 左乘: H = (I - 2vv^T) H */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = k + 1; i < n; ++i)
                dot += v[i] * H[i][j];
            for (int i = k + 1; i < n; ++i)
                H[i][j] -= 2.0 * v[i] * dot;
        }

        /* 右乘: H = H (I - 2vv^T) */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j)
                dot += H[i][j] * v[j];
            for (int j = k + 1; j < n; ++j)
                H[i][j] -= 2.0 * dot * v[j];
        }

        /* 累积正交矩阵: Q = Q (I - 2vv^T) */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j)
                dot += Q[i][j] * v[j];
            for (int j = k + 1; j < n; ++j)
                Q[i][j] -= 2.0 * dot * v[j];
        }
    }

    m_stats.totalReductions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalReductions;

    emit reductionCompleted(n);
    return {Q, H};
}

/** @brief 重置统计 */
void HessenbergReduction::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
