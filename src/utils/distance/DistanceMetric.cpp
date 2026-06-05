/**
 * @file DistanceMetric.cpp
 * @brief 距离度量库实现
 */

#include "utils/distance/DistanceMetric.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DistanceMetric::DistanceMetric(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 欧氏距离 */
double DistanceMetric::euclidean(const QVector<double>& a,
                                  const QVector<double>& b) const
{
    int n = qMin(a.size(), b.size());
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/** @brief 曼哈顿距离 */
double DistanceMetric::manhattan(const QVector<double>& a,
                                  const QVector<double>& b) const
{
    int n = qMin(a.size(), b.size());
    double sum = 0.0;
    for (int i = 0; i < n; ++i)
        sum += qAbs(a[i] - b[i]);
    return sum;
}

/** @brief 切比雪夫距离 */
double DistanceMetric::chebyshev(const QVector<double>& a,
                                  const QVector<double>& b) const
{
    int n = qMin(a.size(), b.size());
    double maxDist = 0.0;
    for (int i = 0; i < n; ++i)
        maxDist = qMax(maxDist, qAbs(a[i] - b[i]));
    return maxDist;
}

/** @brief 闵可夫斯基距离 */
double DistanceMetric::minkowski(const QVector<double>& a,
                                  const QVector<double>& b, double p) const
{
    int n = qMin(a.size(), b.size());
    double sum = 0.0;
    for (int i = 0; i < n; ++i)
        sum += qPow(qAbs(a[i] - b[i]), p);
    return qPow(sum, 1.0 / p);
}

/** @brief 余弦相似度 */
double DistanceMetric::cosineSimilarity(const QVector<double>& a,
                                         const QVector<double>& b) const
{
    int n = qMin(a.size(), b.size());
    double dot = 0.0, nA = 0.0, nB = 0.0;
    for (int i = 0; i < n; ++i) {
        dot += a[i] * b[i];
        nA += a[i] * a[i];
        nB += b[i] * b[i];
    }
    double denom = qSqrt(nA) * qSqrt(nB);
    return (denom > 1e-15) ? dot / denom : 0.0;
}

/** @brief 余弦距离 */
double DistanceMetric::cosineDistance(const QVector<double>& a,
                                       const QVector<double>& b) const
{
    return 1.0 - cosineSimilarity(a, b);
}

/** @brief 汉明距离 */
int DistanceMetric::hamming(const QVector<int>& a,
                             const QVector<int>& b) const
{
    int n = qMin(a.size(), b.size());
    int dist = 0;
    for (int i = 0; i < n; ++i)
        if (a[i] != b[i]) ++dist;
    return dist;
}

/** @brief Jaccard系数 */
double DistanceMetric::jaccard(const QVector<int>& setA,
                                const QVector<int>& setB) const
{
    int intersection = 0;
    for (int v : setA) {
        if (setB.contains(v)) ++intersection;
    }
    int unionSize = setA.size() + setB.size() - intersection;
    return (unionSize > 0) ? static_cast<double>(intersection) / unionSize : 0.0;
}

/** @brief 编辑距离 */
int DistanceMetric::editDistance(const QByteArray& s1,
                                  const QByteArray& s2) const
{
    int m = s1.size();
    int n = s2.size();
    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));

    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1,
                                 dp[i][j - 1] + 1,
                                 dp[i - 1][j - 1] + cost});
        }
    }
    return dp[m][n];
}

/** @brief 重置统计 */
void DistanceMetric::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
