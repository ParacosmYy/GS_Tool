#include "utils/bayes3/NaiveBayesClassifier.h"
#include <QElapsedTimer>
#include <QtMath>

NaiveBayesClassifier::NaiveBayesClassifier(QObject* parent)
    : QObject(parent) {}

bool NaiveBayesClassifier::train(const QVector<QVector<double>>& data,
                                  const QVector<int>& labels, Mode mode) {
    QElapsedTimer timer; timer.start();
    m_mode = mode;
    int n = data.size();
    if (n == 0 || labels.size() != n) return false;
    m_totalSamples = n;
    int d = data[0].size();

    /* 统计类别先验 */
    QMap<int, int> classCounts;
    for (int label : labels) classCounts[label]++;
    m_classPriors.clear();
    for (auto it = classCounts.begin(); it != classCounts.end(); ++it)
        m_classPriors[it.key()] = static_cast<double>(it.value()) / n;

    if (mode == Mode::Gaussian) {
        m_gaussianParams.clear();
        for (auto it = classCounts.begin(); it != classCounts.end(); ++it) {
            int label = it.key();
            int cnt = it.value();
            QVector<ClassParams> params(d);
            for (int f = 0; f < d; ++f) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i)
                    if (labels[i] == label) sum += data[i][f];
                params[f].mean = sum / cnt;
                double var = 0.0;
                for (int i = 0; i < n; ++i)
                    if (labels[i] == label) {
                        double diff = data[i][f] - params[f].mean;
                        var += diff * diff;
                    }
                params[f].variance = var / cnt + 1e-10;
            }
            m_gaussianParams[label] = params;
        }
    }
    m_timeSum += static_cast<double>(timer.elapsed());
    ++m_stats.totalTrainings;
    double total = static_cast<double>(m_stats.totalTrainings + m_stats.totalPredictions);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
    emit trainingCompleted(n);
    return true;
}

int NaiveBayesClassifier::predict(const QVector<double>& sample) const {
    auto proba = predictProba(sample);
    if (proba.isEmpty()) return -1;
    int best = proba[0].first;
    double bestP = proba[0].second;
    for (const auto& p : proba)
        if (p.second > bestP) { bestP = p.second; best = p.first; }
    return best;
}

QVector<QPair<int, double>> NaiveBayesClassifier::predictProba(
    const QVector<double>& sample) const {
    QVector<QPair<int, double>> results;
    QVector<QPair<int, double>> logProbs;
    double maxLog = -1e18;
    for (auto it = m_classPriors.begin(); it != m_classPriors.end(); ++it) {
        int label = it.key();
        double lp = qLn(it.value());
        if (m_mode == Mode::Gaussian && m_gaussianParams.contains(label)) {
            const auto params = m_gaussianParams.value(label);
            for (int f = 0; f < qMin(sample.size(), params.size()); ++f)
                lp += gaussianLogProb(sample[f], params[f].mean, params[f].variance);
        }
        logProbs.append({label, lp});
        if (lp > maxLog) maxLog = lp;
    }
    double sum = 0.0;
    for (auto& p : logProbs) { p.second = qExp(p.second - maxLog); sum += p.second; }
    for (auto& p : logProbs) p.second /= qMax(sum, 1e-15);
    return logProbs;
}

double NaiveBayesClassifier::gaussianLogProb(double x, double mean, double var) const {
    double diff = x - mean;
    return -0.5 * qLn(2.0 * M_PI * var) - diff * diff / (2.0 * var);
}

double NaiveBayesClassifier::multinomialLogProb(double x, int label, int featIdx) const {
    Q_UNUSED(x); Q_UNUSED(label); Q_UNUSED(featIdx);
    return 0.0;
}

void NaiveBayesClassifier::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
