/**
 * @file GaussianNaiveBayes.cpp
 * @brief 高斯朴素贝叶斯分类器实现 — Laplace平滑
 */

#include "utils/cluster5/GaussianNaiveBayes.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>
#include <random>

GaussianNaiveBayes::GaussianNaiveBayes(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

bool GaussianNaiveBayes::train(const QVector<QVector<double>>& features,
                                 const QVector<int>& labels,
                                 double laplaceSmoothing)
{
    QElapsedTimer timer;
    timer.start();

    int nSamples = features.size();
    if (nSamples == 0 || labels.size() != nSamples) return false;

    /* 确定特征维度 */
    m_numFeatures = features[0].size();
    for (const auto& row : features) {
        if (row.size() != m_numFeatures) return false;
    }

    /* 统计各类别样本数 */
    QMap<int, int> classCounts;
    for (int label : labels) {
        ++classCounts[label];
    }

    /* 提取类别列表并排序 */
    m_classes = classCounts.keys();
    std::sort(m_classes.begin(), m_classes.end());

    /* 计算先验概率 P(y) */
    for (int cls : m_classes) {
        m_classPriors[cls] = static_cast<double>(classCounts[cls]) / nSamples;
    }

    /* 计算各类各特征的均值和方差 */
    for (int cls : m_classes) {
        QVector<double> sum(m_numFeatures, 0.0);
        QVector<double> sumSq(m_numFeatures, 0.0);
        int count = classCounts[cls];

        for (int i = 0; i < nSamples; ++i) {
            if (labels[i] != cls) continue;
            for (int f = 0; f < m_numFeatures; ++f) {
                sum[f] += features[i][f];
                sumSq[f] += features[i][f] * features[i][f];
            }
        }

        QVector<double> means(m_numFeatures);
        QVector<double> variances(m_numFeatures);
        for (int f = 0; f < m_numFeatures; ++f) {
            means[f] = sum[f] / count;
            /* 总体方差 = E[X^2] - (E[X])^2, 加Laplace平滑 */
            double rawVar = (sumSq[f] / count) - means[f] * means[f];
            variances[f] = qMax(laplaceSmoothing, rawVar);
        }

        m_classMeans[cls] = means;
        m_classVariances[cls] = variances;
    }

    m_stats.totalSamplesTrained += nSamples;
    m_stats.totalFeatures = m_numFeatures;
    m_stats.totalClasses = m_classes.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSamplesTrained > 0)
        ? m_timeSum / m_stats.totalSamplesTrained : 0.0;

    return true;
}

GaussianNaiveBayes::Prediction GaussianNaiveBayes::predict(
    const QVector<double>& features) const
{
    Prediction result;
    if (features.size() != m_numFeatures || m_classes.isEmpty()) return result;

    /* 计算各类别的对数后验概率 */
    double maxLogPosterior = -1e30;
    int bestClass = m_classes[0];

    for (int cls : m_classes) {
        double logPosterior = qLn(m_classPriors[cls]);

        for (int f = 0; f < m_numFeatures; ++f) {
            logPosterior += logGaussianPDF(features[f],
                                           m_classMeans[cls][f],
                                           m_classVariances[cls][f]);
        }

        result.classProbabilities[cls] = logPosterior;
        if (logPosterior > maxLogPosterior) {
            maxLogPosterior = logPosterior;
            bestClass = cls;
        }
    }

    result.predictedClass = bestClass;
    result.logPosterior = maxLogPosterior;

    /* 将对数概率转换为后验概率(softmax) */
    double logSum = 0.0;
    double maxLog = maxLogPosterior;
    for (int cls : m_classes) {
        logSum += qExp(result.classProbabilities[cls] - maxLog);
    }
    logSum = maxLog + qLn(logSum);

    double totalProb = 0.0;
    for (int cls : m_classes) {
        double prob = qExp(result.classProbabilities[cls] - logSum);
        result.classProbabilities[cls] = prob;
        totalProb += prob;
    }

    result.confidence = result.classProbabilities[bestClass];

    return result;
}

QVector<GaussianNaiveBayes::Prediction> GaussianNaiveBayes::predictBatch(
    const QVector<QVector<double>>& features) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<Prediction> results;
    results.reserve(features.size());

    for (const auto& sample : features) {
        results.append(predict(sample));
    }

    m_stats.totalPredictions += features.size();
    m_timeSum += timer.elapsed();
    if (m_stats.totalPredictions > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalPredictions;

    return results;
}

GaussianNaiveBayes::CrossValidationResult GaussianNaiveBayes::crossValidate(
    const QVector<QVector<double>>& features,
    const QVector<int>& labels, int k) const
{
    QElapsedTimer timer;
    timer.start();

    CrossValidationResult result;
    int n = features.size();
    if (n == 0 || k <= 0 || k > n) return result;

    QVector<int> indices = shuffleIndices(n);
    int foldSize = n / k;

    QVector<int> allPredicted, allActual;

    for (int fold = 0; fold < k; ++fold) {
        /* 划分训练集和验证集 */
        int valStart = fold * foldSize;
        int valEnd = (fold == k - 1) ? n : valStart + foldSize;

        QVector<QVector<double>> trainFeatures, valFeatures;
        QVector<int> trainLabels, valLabels;

        for (int i = 0; i < n; ++i) {
            int idx = indices[i];
            if (i >= valStart && i < valEnd) {
                valFeatures.append(features[idx]);
                valLabels.append(labels[idx]);
            } else {
                trainFeatures.append(features[idx]);
                trainLabels.append(labels[idx]);
            }
        }

        /* 训练临时模型 */
        GaussianNaiveBayes tempModel;
        if (!tempModel.train(trainFeatures, trainLabels)) continue;

        /* 预测验证集 */
        auto predictions = tempModel.predictBatch(valFeatures);

        for (int i = 0; i < valLabels.size(); ++i) {
            allPredicted.append(predictions[i].predictedClass);
            allActual.append(valLabels[i]);
        }
    }

    /* 计算指标 */
    int correct = 0;
    QMap<int, int> truePos, falsePos, falseNeg, classCorrect, classTotal;

    for (int i = 0; i < allActual.size(); ++i) {
        int actual = allActual[i];
        int predicted = allPredicted[i];
        ++classTotal[actual];

        if (actual == predicted) {
            ++correct;
            ++truePos[actual];
            ++classCorrect[actual];
        } else {
            ++falsePos[predicted];
            ++falseNeg[actual];
        }
    }

    result.accuracy = (n > 0) ? static_cast<double>(correct) / n : 0.0;

    /* 宏平均精确率、召回率、F1 */
    double totalPrecision = 0.0, totalRecall = 0.0;
    int numClasses = m_classes.isEmpty() ? classTotal.size() : m_classes.size();

    for (auto it = classTotal.begin(); it != classTotal.end(); ++it) {
        int cls = it.key();
        double prec = (truePos[cls] + falsePos[cls] > 0)
            ? static_cast<double>(truePos[cls]) / (truePos[cls] + falsePos[cls]) : 0.0;
        double rec = static_cast<double>(truePos[cls]) / qMax(1, it.value());
        totalPrecision += prec;
        totalRecall += rec;

        result.perClassAccuracy[cls] = (it.value() > 0)
            ? static_cast<double>(classCorrect[cls]) / it.value() : 0.0;
    }

    result.precision = (numClasses > 0) ? totalPrecision / numClasses : 0.0;
    result.recall = (numClasses > 0) ? totalRecall / numClasses : 0.0;
    result.f1Score = (result.precision + result.recall > 0)
        ? 2.0 * result.precision * result.recall / (result.precision + result.recall) : 0.0;

    ++m_stats.totalCrossValidations;
    m_timeSum += timer.elapsed();

    return result;
}

QVector<double> GaussianNaiveBayes::featureImportance() const
{
    if (m_classes.size() < 2 || m_numFeatures == 0) return {};

    /* 基于类间方差比: 越大说明该特征区分能力越强 */
    QVector<double> importance(m_numFeatures, 0.0);

    /* 计算总体均值 */
    QVector<double> globalMean(m_numFeatures, 0.0);
    for (int cls : m_classes) {
        double prior = m_classPriors[cls];
        for (int f = 0; f < m_numFeatures; ++f) {
            globalMean[f] += prior * m_classMeans[cls][f];
        }
    }

    /* 计算类间散度 / 类内散度 */
    for (int f = 0; f < m_numFeatures; ++f) {
        double betweenClassVar = 0.0;
        double withinClassVar = 0.0;

        for (int cls : m_classes) {
            double diff = m_classMeans[cls][f] - globalMean[f];
            betweenClassVar += m_classPriors[cls] * diff * diff;
            withinClassVar += m_classPriors[cls] * m_classVariances[cls][f];
        }

        importance[f] = (withinClassVar > 1e-15)
            ? betweenClassVar / withinClassVar : 0.0;
    }

    /* 归一化到 [0, 1] */
    double maxImp = *std::max_element(importance.begin(), importance.end());
    if (maxImp > 0) {
        for (auto& v : importance) v /= maxImp;
    }

    return importance;
}

QMap<int, QPair<QVector<double>, QVector<double>>> GaussianNaiveBayes::classParameters() const
{
    QMap<int, QPair<QVector<double>, QVector<double>>> params;
    for (int cls : m_classes) {
        params[cls] = {m_classMeans[cls], m_classVariances[cls]};
    }
    return params;
}

double GaussianNaiveBayes::accuracy(const QVector<QVector<double>>& features,
                                      const QVector<int>& labels) const
{
    if (features.size() != labels.size()) return -1.0;

    int correct = 0;
    for (int i = 0; i < features.size(); ++i) {
        auto pred = predict(features[i]);
        if (pred.predictedClass == labels[i]) ++correct;
    }

    return (features.size() > 0)
        ? static_cast<double>(correct) / features.size() : 0.0;
}

QVector<int> GaussianNaiveBayes::classes() const
{
    return m_classes;
}

QMap<int, double> GaussianNaiveBayes::classPriors() const
{
    return m_classPriors;
}

double GaussianNaiveBayes::gaussianPDF(double x, double mean, double variance) const
{
    if (variance <= 0) return 0.0;
    double diff = x - mean;
    return (1.0 / qSqrt(2.0 * M_PI * variance))
           * qExp(-0.5 * diff * diff / variance);
}

double GaussianNaiveBayes::logGaussianPDF(double x, double mean, double variance) const
{
    if (variance <= 0) return -1e30;
    double diff = x - mean;
    return -0.5 * qLn(2.0 * M_PI * variance) - 0.5 * diff * diff / variance;
}

QVector<int> GaussianNaiveBayes::shuffleIndices(int n) const
{
    QVector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    return indices;
}

GaussianNaiveBayes::Stats GaussianNaiveBayes::stats() const
{
    return m_stats;
}

void GaussianNaiveBayes::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
