/**
 * @file BayesClassifier.cpp
 * @brief 贝叶斯分类器实现
 */

#include "utils/bayes/BayesClassifier.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <set>

BayesClassifier::BayesClassifier(QObject* parent)
    : QObject(parent), m_modelType(ModelType::Gaussian),
      m_smoothing(1.0), m_featureCount(0), m_timeSum(0.0) {}

void BayesClassifier::setModelType(ModelType type) { m_modelType = type; }
void BayesClassifier::setSmoothing(double alpha) { m_smoothing = qMax(1e-10, alpha); }

void BayesClassifier::train(const QVector<QVector<double>>& features, const QVector<int>& labels)
{
    QElapsedTimer timer;
    timer.start();

    if (features.isEmpty() || labels.size() != features.size()) return;

    m_featureCount = features[0].size();

    switch (m_modelType) {
    case ModelType::Gaussian:   trainGaussian(features, labels); break;
    case ModelType::Multinomial: trainMultinomial(features, labels); break;
    case ModelType::Bernoulli:  trainBernoulli(features, labels); break;
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;

    std::set<int> classes;
    for (int l : labels) classes.insert(l);
    emit trainingComplete(classes.size(), features.size());
}

void BayesClassifier::trainGaussian(const QVector<QVector<double>>& features, const QVector<int>& labels)
{
    QMap<int, QVector<double>> sums;
    QMap<int, int> counts;

    for (int i = 0; i < features.size(); ++i) {
        int label = labels[i];
        counts[label]++;
        if (!sums.contains(label)) sums[label] = QVector<double>(m_featureCount, 0.0);
        for (int j = 0; j < m_featureCount; ++j) sums[label][j] += features[i][j];
    }

    m_classMeans.clear();
    m_classVariances.clear();
    m_classPrior.clear();

    for (auto it = counts.begin(); it != counts.end(); ++it) {
        int c = it.key();
        int n = it.value();
        m_classPrior[c] = static_cast<double>(n) / features.size();
        m_classMeans[c] = QVector<double>(m_featureCount);
        for (int j = 0; j < m_featureCount; ++j) m_classMeans[c][j] = sums[c][j] / n;

        QVector<double> varSum(m_featureCount, 0.0);
        for (int i = 0; i < features.size(); ++i) {
            if (labels[i] != c) continue;
            for (int j = 0; j < m_featureCount; ++j) {
                double d = features[i][j] - m_classMeans[c][j];
                varSum[j] += d * d;
            }
        }
        m_classVariances[c] = QVector<double>(m_featureCount);
        for (int j = 0; j < m_featureCount; ++j)
            m_classVariances[c][j] = (varSum[j] / n) + 1e-9;
    }
}

void BayesClassifier::trainMultinomial(const QVector<QVector<double>>& features, const QVector<int>& labels)
{
    QMap<int, QVector<double>> featureSums;
    QMap<int, int> counts;

    for (int i = 0; i < features.size(); ++i) {
        int label = labels[i];
        counts[label]++;
        if (!featureSums.contains(label)) featureSums[label] = QVector<double>(m_featureCount, 0.0);
        for (int j = 0; j < m_featureCount; ++j) featureSums[label][j] += qMax(0.0, features[i][j]);
    }

    m_classLogProbs.clear();
    m_classPrior.clear();

    for (auto it = counts.begin(); it != counts.end(); ++it) {
        int c = it.key();
        m_classPrior[c] = static_cast<double>(it.value()) / features.size();
        double totalFeatures = 0.0;
        for (int j = 0; j < m_featureCount; ++j) totalFeatures += featureSums[c][j];

        m_classLogProbs[c] = QVector<double>(m_featureCount);
        for (int j = 0; j < m_featureCount; ++j) {
            m_classLogProbs[c][j] = qLn((featureSums[c][j] + m_smoothing) /
                                          (totalFeatures + m_smoothing * m_featureCount));
        }
    }
}

void BayesClassifier::trainBernoulli(const QVector<QVector<double>>& features, const QVector<int>& labels)
{
    QMap<int, QVector<double>> featurePresent;
    QMap<int, int> counts;

    for (int i = 0; i < features.size(); ++i) {
        int label = labels[i];
        counts[label]++;
        if (!featurePresent.contains(label)) featurePresent[label] = QVector<double>(m_featureCount, 0.0);
        for (int j = 0; j < m_featureCount; ++j)
            if (features[i][j] > 0.5) featurePresent[label][j] += 1.0;
    }

    m_classLogProbs.clear();
    m_classPrior.clear();

    for (auto it = counts.begin(); it != counts.end(); ++it) {
        int c = it.key();
        int n = it.value();
        m_classPrior[c] = static_cast<double>(n) / features.size();
        m_classLogProbs[c] = QVector<double>(m_featureCount);
        for (int j = 0; j < m_featureCount; ++j) {
            double p = (featurePresent[c][j] + m_smoothing) / (n + 2 * m_smoothing);
            m_classLogProbs[c][j] = p;
        }
    }
}

int BayesClassifier::predict(const QVector<double>& feature)
{
    QMap<int, double> probs = predictProba(feature);
    int bestClass = -1;
    double bestProb = -std::numeric_limits<double>::max();
    for (auto it = probs.begin(); it != probs.end(); ++it) {
        if (it.value() > bestProb) { bestProb = it.value(); bestClass = it.key(); }
    }
    ++m_stats.totalClassifications;
    emit classificationDone(bestClass, bestProb);
    return bestClass;
}

QMap<int, double> BayesClassifier::predictProba(const QVector<double>& feature)
{
    QMap<int, double> logProbs;

    for (auto it = m_classPrior.begin(); it != m_classPrior.end(); ++it) {
        int c = it.key();
        double logP = qLn(it.value());

        switch (m_modelType) {
        case ModelType::Gaussian: {
            for (int j = 0; j < m_featureCount && j < feature.size(); ++j) {
                double mean = m_classMeans[c][j];
                double var = m_classVariances[c][j];
                double d = feature[j] - mean;
                logP += -0.5 * qLn(2.0 * M_PI * var) - (d * d) / (2.0 * var);
            }
            break;
        }
        case ModelType::Multinomial: {
            for (int j = 0; j < m_featureCount && j < feature.size(); ++j) {
                logP += m_classLogProbs[c][j] * qMax(0.0, feature[j]);
            }
            break;
        }
        case ModelType::Bernoulli: {
            for (int j = 0; j < m_featureCount && j < feature.size(); ++j) {
                double p = m_classLogProbs[c][j];
                if (feature[j] > 0.5) logP += qLn(p + 1e-15);
                else logP += qLn(1.0 - p + 1e-15);
            }
            break;
        }
        }
        logProbs[c] = logP;
    }

    /* Log-sum-exp归一化 */
    double maxLog = -std::numeric_limits<double>::max();
    for (double v : logProbs) if (v > maxLog) maxLog = v;
    double sumExp = 0.0;
    for (auto it = logProbs.begin(); it != logProbs.end(); ++it) {
        it.value() = qExp(it.value() - maxLog);
        sumExp += it.value();
    }
    for (auto it = logProbs.begin(); it != logProbs.end(); ++it)
        it.value() /= sumExp;

    return logProbs;
}

double BayesClassifier::evaluate(const QVector<QVector<double>>& features, const QVector<int>& labels)
{
    int correct = 0;
    for (int i = 0; i < features.size(); ++i) {
        if (predict(features[i]) == labels[i]) ++correct;
    }
    m_stats.correctClassifications += correct;
    m_stats.accuracy = (m_stats.totalClassifications > 0)
        ? static_cast<double>(m_stats.correctClassifications) / m_stats.totalClassifications : 0.0;
    return static_cast<double>(correct) / features.size();
}

void BayesClassifier::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
