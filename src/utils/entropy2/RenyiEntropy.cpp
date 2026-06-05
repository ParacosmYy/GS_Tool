#include "utils/entropy2/RenyiEntropy.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

RenyiEntropy::RenyiEntropy(QObject* parent) : QObject(parent), m_timeSum(0.0) {}

double RenyiEntropy::renyi(const QVector<double>& p, double alpha) {
    if (alpha <= 0.0 || qAbs(alpha - 1.0) < 1e-10) return shannon(p);
    double sum = 0.0;
    for (double pi : p) if (pi > 0.0) sum += qPow(pi, alpha);
    double h = qLn(qMax(sum, 1e-300)) / (1.0 - alpha);
    emit computationCompleted(h);
    return h;
}

double RenyiEntropy::tsallis(const QVector<double>& p, double alpha) {
    if (qAbs(alpha - 1.0) < 1e-10) return shannon(p);
    double sum = 0.0;
    for (double pi : p) if (pi > 0.0) sum += qPow(pi, alpha);
    return (sum - 1.0) / (1.0 - alpha);
}

double RenyiEntropy::shannon(const QVector<double>& p) {
    double h = 0.0;
    for (double pi : p) if (pi > 0.0) h -= pi * qLn(pi);
    return h;
}

double RenyiEntropy::collision(const QVector<double>& p) { return renyi(p, 2.0); }

QVector<double> RenyiEntropy::estimateProbabilities(
    const QVector<double>& data, int bins) const {
    if (data.isEmpty() || bins <= 0) return QVector<double>();
    double mn = *std::min_element(data.begin(), data.end());
    double mx = *std::max_element(data.begin(), data.end());
    double range = mx - mn;
    if (range < 1e-15) range = 1.0;
    QVector<double> counts(bins, 0.0);
    for (double v : data) {
        int idx = static_cast<int>((v - mn) / range * (bins - 1));
        idx = qBound(0, idx, bins - 1);
        counts[idx] += 1.0;
    }
    double total = static_cast<double>(data.size());
    for (auto& c : counts) c /= total;
    return counts;
}

void RenyiEntropy::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
