/**
 * @file FastDtw.cpp
 * @brief 快速DTW距离计算实现
 */

#include "utils/dtw2/FastDtw.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

FastDtw::FastDtw(QObject* parent)
    : QObject(parent), m_mode(Mode::Standard), m_radius(1),
      m_metric("euclidean"), m_timeSum(0.0) {}

void FastDtw::setMode(Mode m) { m_mode = m; }
void FastDtw::setRadius(int r) { m_radius = qMax(0, r); }
void FastDtw::setDistanceMetric(const QString& metric) { m_metric = metric; }

FastDtw::DtwResult FastDtw::compute(const QVector<double>& seq1, const QVector<double>& seq2)
{
    QElapsedTimer timer;
    timer.start();

    DtwResult result;
    if (seq1.isEmpty() || seq2.isEmpty()) return result;

    switch (m_mode) {
    case Mode::Standard:   result = standardDtw(seq1, seq2); break;
    case Mode::FastDtw:    result = fastDtwImpl(seq1, seq2, m_radius); break;
    case Mode::Constrained: result = constrainedDtw(seq1, seq2); break;
    }

    double elapsed = timer.elapsed();
    result.averageProcessingTimeMs = elapsed;

    ++m_stats.totalComputations;
    double distSum = m_stats.avgDistance * (m_stats.totalComputations - 1) + result.distance;
    m_stats.avgDistance = distSum / m_stats.totalComputations;
    double pathSum = m_stats.avgPathLength * (m_stats.totalComputations - 1) + result.warpPath.size();
    m_stats.avgPathLength = pathSum / m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationComplete(result.distance, result.warpPath.size());
    return result;
}

FastDtw::DtwResult FastDtw::standardDtw(const QVector<double>& s1, const QVector<double>& s2)
{
    DtwResult result;
    int n = s1.size(), m = s2.size();

    QVector<QVector<double>> dp(n + 1, QVector<double>(m + 1, std::numeric_limits<double>::max()));
    QVector<QVector<int>> path(n + 1, QVector<int>(m + 1, 0));

    dp[0][0] = 0.0;
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            double cost = pointDistance(s1[i - 1], s2[j - 1]);
            double minPrev = dp[i-1][j];
            int from = 1;
            if (dp[i][j-1] < minPrev) { minPrev = dp[i][j-1]; from = 2; }
            if (dp[i-1][j-1] < minPrev) { minPrev = dp[i-1][j-1]; from = 3; }
            dp[i][j] = cost + minPrev;
            path[i][j] = from;
        }
    }

    result.distance = dp[n][m];
    int pathLen = qMax(n, m);
    result.normalizedDistance = result.distance / pathLen;
    result.warpPath = traceback(path, n, m);
    return result;
}

FastDtw::DtwResult FastDtw::fastDtwImpl(const QVector<double>& s1, const QVector<double>& s2, int radius)
{
    DtwResult result;
    int n = s1.size(), m = s2.size();

    if (n <= radius + 2 || m <= radius + 2) return standardDtw(s1, s2);

    /* 降采样 */
    int halfN = n / 2, halfM = m / 2;
    QVector<double> shrunk1(halfN), shrunk2(halfM);
    for (int i = 0; i < halfN; ++i) shrunk1[i] = (s1[2*i] + s1[2*i+1]) / 2.0;
    for (int j = 0; j < halfM; ++j) shrunk2[j] = (s2[2*j] + s2[2*j+1]) / 2.0;

    /* 递归计算 */
    DtwResult lowRes = fastDtwImpl(shrunk1, shrunk2, radius);

    /* 将路径投影回原始分辨率并扩展搜索窗口 */
    QList<QPair<int, int>> window;
    for (const auto& pt : lowRes.warpPath) {
        int x = pt.first * 2, y = pt.second * 2;
        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dy = -radius; dy <= radius; ++dy) {
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && nx < n && ny >= 0 && ny < m)
                    window.append({nx, ny});
            }
        }
    }
    std::sort(window.begin(), window.end());
    window.erase(std::unique(window.begin(), window.end()), window.end());

    /* 在约束窗口内做DTW */
    if (window.size() < static_cast<size_t>(n) * m * 0.8) {
        QVector<QVector<double>> dp(n + 1, QVector<double>(m + 1, std::numeric_limits<double>::max()));
        QVector<QVector<int>> path(n + 1, QVector<int>(m + 1, 0));
        dp[0][0] = 0.0;

        for (const auto& pt : window) {
            int i = pt.first + 1, j = pt.second + 1;
            double cost = pointDistance(s1[pt.first], s2[pt.second]);
            double minPrev = dp[i-1][j];
            int from = 1;
            if (dp[i][j-1] < minPrev) { minPrev = dp[i][j-1]; from = 2; }
            if (dp[i-1][j-1] < minPrev) { minPrev = dp[i-1][j-1]; from = 3; }
            dp[i][j] = cost + minPrev;
            path[i][j] = from;
        }

        result.distance = dp[n][m];
        result.normalizedDistance = result.distance / qMax(n, m);
        result.warpPath = traceback(path, n, m);
    } else {
        result = standardDtw(s1, s2);
    }
    return result;
}

FastDtw::DtwResult FastDtw::constrainedDtw(const QVector<double>& s1, const QVector<double>& s2)
{
    DtwResult result;
    int n = s1.size(), m = s2.size();
    int w = qMax(m_radius, qAbs(n - m));

    QVector<QVector<double>> dp(n + 1, QVector<double>(m + 1, std::numeric_limits<double>::max()));
    QVector<QVector<int>> path(n + 1, QVector<int>(m + 1, 0));
    dp[0][0] = 0.0;

    for (int i = 1; i <= n; ++i) {
        int jMin = qMax(1, i - w);
        int jMax = qMin(m, i + w);
        for (int j = jMin; j <= jMax; ++j) {
            double cost = pointDistance(s1[i-1], s2[j-1]);
            double minPrev = dp[i-1][j];
            int from = 1;
            if (dp[i][j-1] < minPrev) { minPrev = dp[i][j-1]; from = 2; }
            if (dp[i-1][j-1] < minPrev) { minPrev = dp[i-1][j-1]; from = 3; }
            dp[i][j] = cost + minPrev;
            path[i][j] = from;
        }
    }

    result.distance = dp[n][m];
    result.normalizedDistance = result.distance / qMax(n, m);
    result.warpPath = traceback(path, n, m);
    return result;
}

double FastDtw::pointDistance(double a, double b) const
{
    if (m_metric == "manhattan") return qAbs(a - b);
    return (a - b) * (a - b); // euclidean squared
}

QList<QPair<int, int>> FastDtw::traceback(const QVector<QVector<int>>& path, int n, int m) const
{
    QList<QPair<int, int>> result;
    int i = n, j = m;
    while (i > 0 && j > 0) {
        result.prepend({i - 1, j - 1});
        switch (path[i][j]) {
        case 1: --i; break;
        case 2: --j; break;
        case 3: --i; --j; break;
        default: --i; --j; break;
        }
    }
    return result;
}

void FastDtw::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
