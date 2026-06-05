#include "utils/cluster71/KMeans8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

KMeans8::KMeans8(QObject* parent) : QObject(parent) {}
void KMeans8::setNumClusters(int k) { m_k = qBound(2, k, 1000); }
void KMeans8::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void KMeans8::setDistanceMetric(const QString& metric) { if (metric == "euclidean" || metric == "manhattan") m_metric = metric; }

QVector<int> KMeans8::cluster(const QVector<QVector<double>>& points) {
    QElapsedTimer timer; timer.start();
    const int N = points.size();
    QVector<int> labels(N, 0);
    if (N == 0 || m_k <= 0) { m_timeSum += timer.elapsed(); return labels; }
    const int D = points[0].size();
    m_centroids.clear();
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_int_distribution<int> uid(0, N - 1);
    m_centroids.append(points[uid(gen)]);
    for (int c = 1; c < m_k; ++c) {
        QVector<double> minD(N, 1e18);
        for (int i = 0; i < N; ++i) for (const auto& cent : m_centroids) { double d = distance(points[i], cent); minD[i] = qMin(minD[i], d*d); }
        double tot = 0; for (double d : minD) tot += d;
        std::uniform_real_distribution<double> urd(0, tot); double r = urd(gen), cs = 0;
        for (int i = 0; i < N; ++i) { cs += minD[i]; if (cs >= r) { m_centroids.append(points[i]); break; } }
    }
    for (int it = 0; it < m_maxIter; ++it) {
        bool ch = false;
        for (int i = 0; i < N; ++i) { double bd = 1e18; int bc = 0; for (int c = 0; c < m_k; ++c) { double d = distance(points[i], m_centroids[c]); if (d < bd) { bd = d; bc = c; } } if (labels[i] != bc) { labels[i] = bc; ch = true; } }
        if (!ch) break;
        QVector<QVector<double>> nc(m_k, QVector<double>(D, 0)); QVector<int> cnt(m_k, 0);
        for (int i = 0; i < N; ++i) { cnt[labels[i]]++; for (int d = 0; d < D; ++d) nc[labels[i]][d] += points[i][d]; }
        for (int c = 0; c < m_k; ++c) if (cnt[c] > 0) for (int d = 0; d < D; ++d) nc[c][d] /= cnt[c]; else nc[c] = m_centroids[c];
        m_centroids = nc;
    }
    m_inertia = 0; for (int i = 0; i < N; ++i) { double d = distance(points[i], m_centroids[labels[i]]); m_inertia += d*d; }
    qint64 elapsed = timer.elapsed(); m_stats.totalClusterings++; m_stats.totalPoints += N; m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
    emit clusteringCompleted(m_k, m_inertia);
    return labels;
}

double KMeans8::distance(const QVector<double>& a, const QVector<double>& b) const {
    double d = 0; int sz = qMin(a.size(), b.size());
    if (m_metric == "manhattan") { for (int i = 0; i < sz; ++i) d += qAbs(a[i]-b[i]); }
    else { for (int i = 0; i < sz; ++i) { double df = a[i]-b[i]; d += df*df; } d = qSqrt(d); }
    return d;
}
void KMeans8::resetStatistics() { m_stats = Stats(); m_timeSum = 0; }
