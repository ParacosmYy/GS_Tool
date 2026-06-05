#include "utils/crossval/CrossValidator.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

CrossValidator::CrossValidator(QObject* parent) : QObject(parent) {}

CrossValidator::ValidationResult CrossValidator::validate(
    ModelEvaluator evaluator, const QVector<QVector<double>>& data,
    const QVector<int>& labels, int k) {
    QElapsedTimer timer; timer.start();
    int n = data.size();
    ValidationResult result;
    if (n == 0 || k <= 0) return result;

    /* 随机打乱索引 */
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;
    std::random_device rd; std::mt19937 g(rd()); std::shuffle(indices.begin(), indices.end(), g);

    result.foldScores.resize(k);
    double sum = 0.0;
    int foldSize = n / k;

    for (int f = 0; f < k; ++f) {
        int start = f * foldSize;
        int end = (f == k - 1) ? n : (f + 1) * foldSize;

        QVector<QVector<double>> trainData, testData;
        QVector<int> trainLabels, testLabels;
        for (int i = 0; i < n; ++i) {
            if (i >= start && i < end) {
                testData.append(data[indices[i]]);
                testLabels.append(labels[indices[i]]);
            } else {
                trainData.append(data[indices[i]]);
                trainLabels.append(labels[indices[i]]);
            }
        }
        double score = evaluator(trainData, trainLabels, testData, testLabels);
        result.foldScores[f] = score;
        sum += score;
        emit foldCompleted(f, score);
    }

    result.meanScore = sum / k;
    double var = 0.0;
    for (double s : result.foldScores) var += (s - result.meanScore) * (s - result.meanScore);
    result.stdScore = qSqrt(var / k);

    m_timeSum += static_cast<double>(timer.elapsed());
    ++m_stats.totalValidations;
    m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_stats.totalValidations);
    return result;
}

CrossValidator::ValidationResult CrossValidator::leaveOneOut(
    ModelEvaluator evaluator, const QVector<QVector<double>>& data,
    const QVector<int>& labels) {
    return validate(evaluator, data, labels, data.size());
}

void CrossValidator::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
