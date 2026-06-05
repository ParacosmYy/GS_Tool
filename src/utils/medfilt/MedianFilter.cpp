#include "utils/medfilt/MedianFilter.h"
#include <QElapsedTimer>
#include <algorithm>
MedianFilter::MedianFilter(QObject* p) : QObject(p), m_windowSize(5), m_timeSum(0.0) {}
void MedianFilter::setWindowSize(int s) { m_windowSize = qMax(3, s | 1); }
double MedianFilter::quickSelect(QVector<double>& d, int k) {
    int lo = 0, hi = d.size() - 1;
    while (lo < hi) {
        double piv = d[(lo+hi)/2]; int i = lo, j = hi;
        while (i <= j) { while (d[i] < piv) ++i; while (d[j] > piv) --j; if (i <= j) { std::swap(d[i],d[j]); ++i; --j; } }
        if (k <= j) hi = j; else if (k >= i) lo = i; else break;
    }
    return d[k];
}
QVector<double> MedianFilter::apply(const QVector<double>& sig) {
    QElapsedTimer t; t.start();
    int n = sig.size(), half = m_windowSize / 2;
    QVector<double> out(n);
    for (int i = 0; i < n; ++i) {
        QVector<double> w;
        for (int j = -half; j <= half; ++j) w << sig[qBound(0, i+j, n-1)];
        out[i] = quickSelect(w, w.size() / 2);
    }
    m_stats.totalApplications++; m_timeSum += t.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalApplications;
    emit applicationCompleted(n); return out;
}
QVector<double> MedianFilter::apply2d(const QVector<double>& mat, int rows, int cols) {
    QElapsedTimer t; t.start();
    QVector<double> out(rows*cols); int half = m_windowSize / 2;
    for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
        QVector<double> w;
        for (int dr = -half; dr <= half; ++dr) for (int dc = -half; dc <= half; ++dc)
            w << mat[qBound(0,r+dr,rows-1)*cols+qBound(0,c+dc,cols-1)];
        out[r*cols+c] = quickSelect(w, w.size()/2);
    }
    m_stats.totalApplications++; m_timeSum += t.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalApplications;
    emit applicationCompleted(rows*cols); return out;
}
void MedianFilter::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
