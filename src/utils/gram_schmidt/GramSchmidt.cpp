/**
 * @file GramSchmidt.cpp
 * @brief Gram-Schmidt正交化实现 — 经典与修正算法
 */

#include "utils/gram_schmidt/GramSchmidt.h"

#include <QElapsedTimer>
#include <QtMath>

#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
GramSchmidt::GramSchmidt(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 经典Gram-Schmidt正交化
 *  @param vectors 输入向量组
 *  @return 正交化后的标准正交基 */
QVector<QVector<double>> GramSchmidt::orthogonalize(
    const QVector<QVector<double>>& vectors)
{
    QElapsedTimer timer;
    timer.start();

    int m = vectors.size();
    if (m == 0) return {};

    int n = vectors[0].size();

    /* 复制输入 */
    QVector<QVector<double>> u(m);
    for (int i = 0; i < m; ++i) {
        u[i] = vectors[i];
    }

    /* CGS: 对每个向量减去其在所有前序基向量上的投影 */
    QVector<QVector<double>> result(m);
    for (int i = 0; i < m; ++i) {
        QVector<double> v = u[i];

        for (int j = 0; j < i; ++j) {
            double proj = dotProduct(v, result[j]);
            for (int k = 0; k < n; ++k) {
                v[k] -= proj * result[j][k];
            }
        }

        double nrm = norm(v);
        if (nrm < 1e-15) {
            /* 线性相关: 返回零向量 */
            result[i].resize(n, 0.0);
            continue;
        }

        result[i] = scale(v, 1.0 / nrm);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalOrthogonalizations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOrthogonalizations);

    emit orthogonalizationCompleted(m);
    return result;
}

/** @brief 修正Gram-Schmidt正交化(数值更稳定)
 *  @param vectors 输入向量组
 *  @return 正交化后的标准正交基 */
QVector<QVector<double>> GramSchmidt::modifiedOrthogonalize(
    const QVector<QVector<double>>& vectors)
{
    QElapsedTimer timer;
    timer.start();

    int m = vectors.size();
    if (m == 0) return {};

    int n = vectors[0].size();

    /* 复制输入到工作数组 */
    QVector<QVector<double>> v(m);
    for (int i = 0; i < m; ++i) {
        v[i] = vectors[i];
    }

    QVector<QVector<double>> result(m);

    for (int i = 0; i < m; ++i) {
        /* MGS: 立即用已正交化的向量逐个减去投影 */
        for (int j = 0; j < i; ++j) {
            double proj = dotProduct(v[i], result[j]);
            v[i] = subtract(v[i], scale(result[j], proj));
        }

        double nrm = norm(v[i]);
        if (nrm < 1e-15) {
            result[i].resize(n, 0.0);
            continue;
        }

        result[i] = scale(v[i], 1.0 / nrm);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalOrthogonalizations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOrthogonalizations);

    emit orthogonalizationCompleted(m);
    return result;
}

/** @brief 向量点积 */
double GramSchmidt::dotProduct(const QVector<double>& a,
                               const QVector<double>& b)
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

/** @brief 向量范数 */
double GramSchmidt::norm(const QVector<double>& v)
{
    return qSqrt(dotProduct(v, v));
}

/** @brief 向量数乘 */
QVector<double> GramSchmidt::scale(const QVector<double>& v, double s)
{
    int n = v.size();
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) {
        r[i] = v[i] * s;
    }
    return r;
}

/** @brief 向量减法 */
QVector<double> GramSchmidt::subtract(const QVector<double>& a,
                                      const QVector<double>& b)
{
    int n = qMin(a.size(), b.size());
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) {
        r[i] = a[i] - b[i];
    }
    return r;
}

/** @brief 重置统计 */
void GramSchmidt::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
