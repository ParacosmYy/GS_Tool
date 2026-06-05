/**
 * @file CrossEntropyLoss.cpp
 * @brief 交叉熵损失函数实现 — 含Softmax集成
 */

#include "utils/loss4/CrossEntropyLoss.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

CrossEntropyLoss::CrossEntropyLoss(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void CrossEntropyLoss::configure(const Config& config)
{
    m_config = config;
}

CrossEntropyLoss::LossResult CrossEntropyLoss::compute(
    const QVector<double>& logits, int classIndex)
{
    QElapsedTimer timer;
    timer.start();

    LossResult result;

    if (logits.isEmpty() || classIndex < 0 || classIndex >= logits.size()) {
        result.loss = 0.0;
        return result;
    }

    switch (m_config.type) {
    case LossType::BinaryCrossEntropy: {
        /* 二元交叉熵: -[y*log(sigma) + (1-y)*log(1-sigma)] */
        double prob = sigmoid(logits[0]);
        double target = static_cast<double>(classIndex);
        double eps = m_config.epsilon;
        result.probabilities = {1.0 - prob, prob};
        result.loss = -(target * qLn(qMax(prob, eps))
                      + (1.0 - target) * qLn(qMax(1.0 - prob, eps)));
        result.predictedClass = (prob >= 0.5) ? 1 : 0;
        result.confidence = qMax(prob, 1.0 - prob);

        if (m_config.computeGradient) {
            result.gradient = {prob - target};
        }
        break;
    }
    case LossType::CategoricalCrossEntropy:
    case LossType::SparseCategoricalCE:
    case LossType::LabelSmoothedCE: {
        /* Softmax + 交叉熵 */
        result.probabilities = softmax(logits);

        /* 构造目标分布 */
        QVector<double> target(logits.size(), 0.0);
        if (m_config.type == LossType::LabelSmoothedCE) {
            target[classIndex] = 1.0;
            target = applyLabelSmoothing(target, m_config.labelSmoothing);
        } else {
            target[classIndex] = 1.0;
        }

        /* 计算损失: -sum(target * log(prob)) */
        double loss = 0.0;
        for (int i = 0; i < result.probabilities.size(); ++i) {
            double p = qMax(result.probabilities[i], m_config.epsilon);
            loss -= target[i] * qLn(p);
        }
        result.loss = loss;

        /* 预测类别 */
        double maxProb = -1.0;
        for (int i = 0; i < result.probabilities.size(); ++i) {
            if (result.probabilities[i] > maxProb) {
                maxProb = result.probabilities[i];
                result.predictedClass = i;
            }
        }
        result.confidence = maxProb;

        /* 梯度: softmax + CE的梯度简化为 (prob - target) */
        if (m_config.computeGradient) {
            result.gradient.resize(logits.size());
            for (int i = 0; i < logits.size(); ++i) {
                result.gradient[i] = result.probabilities[i] - target[i];
            }
        }
        break;
    }
    }

    /* 困惑度 */
    result.perplexity = qExp(result.loss);

    /* 更新统计 */
    ++m_stats.totalLossComputations;
    ++m_stats.totalSamples;
    m_stats.cumulativeLoss += result.loss;
    if (result.loss < m_stats.minLoss) m_stats.minLoss = result.loss;
    if (result.loss > m_stats.maxLoss) m_stats.maxLoss = result.loss;
    if (result.predictedClass == classIndex) {
        ++m_stats.totalCorrectPredictions;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalLossComputations;

    emit lossComputed(result.loss, result.perplexity);
    return result;
}

CrossEntropyLoss::LossResult CrossEntropyLoss::computeMultiTarget(
    const QVector<double>& logits,
    const QVector<double>& targetProbs)
{
    QElapsedTimer timer;
    timer.start();

    LossResult result;

    if (logits.size() != targetProbs.size() || logits.isEmpty()) {
        return result;
    }

    result.probabilities = softmax(logits);

    /* 计算损失: -sum(target * log(prob)) */
    double loss = 0.0;
    for (int i = 0; i < result.probabilities.size(); ++i) {
        double p = qMax(result.probabilities[i], m_config.epsilon);
        loss -= targetProbs[i] * qLn(p);
    }
    result.loss = loss;
    result.perplexity = qExp(result.loss);

    /* 预测类别 */
    double maxProb = -1.0;
    for (int i = 0; i < result.probabilities.size(); ++i) {
        if (result.probabilities[i] > maxProb) {
            maxProb = result.probabilities[i];
            result.predictedClass = i;
        }
    }
    result.confidence = maxProb;

    /* 梯度 */
    if (m_config.computeGradient) {
        result.gradient.resize(logits.size());
        for (int i = 0; i < logits.size(); ++i) {
            result.gradient[i] = result.probabilities[i] - targetProbs[i];
        }
    }

    ++m_stats.totalLossComputations;
    ++m_stats.totalSamples;
    m_stats.cumulativeLoss += loss;
    if (loss < m_stats.minLoss) m_stats.minLoss = loss;
    if (loss > m_stats.maxLoss) m_stats.maxLoss = loss;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalLossComputations;

    emit lossComputed(result.loss, result.perplexity);
    return result;
}

double CrossEntropyLoss::computeBatch(
    const QVector<QVector<double>>& batchLogits,
    const QVector<int>& labels)
{
    QElapsedTimer timer;
    timer.start();

    if (batchLogits.size() != labels.size() || batchLogits.isEmpty()) {
        return 0.0;
    }

    double totalLoss = 0.0;
    int correctCount = 0;

    for (int i = 0; i < batchLogits.size(); ++i) {
        LossResult r = compute(batchLogits[i], labels[i]);
        totalLoss += r.loss;
        if (r.predictedClass == labels[i]) ++correctCount;
    }

    double avgLoss = totalLoss / batchLogits.size();
    double acc = static_cast<double>(correctCount) / batchLogits.size();

    emit batchCompleted(avgLoss, acc);
    return avgLoss;
}

QVector<double> CrossEntropyLoss::softmax(
    const QVector<double>& logits) const
{
    if (logits.isEmpty()) return {};

    /* 数值稳定: 减去最大值 */
    double maxVal = *std::max_element(logits.begin(), logits.end());

    QVector<double> exps(logits.size());
    double sumExp = 0.0;
    for (int i = 0; i < logits.size(); ++i) {
        exps[i] = qExp(logits[i] - maxVal);
        sumExp += exps[i];
    }

    QVector<double> probs(logits.size());
    for (int i = 0; i < logits.size(); ++i) {
        probs[i] = exps[i] / qMax(sumExp, m_config.epsilon);
    }
    return probs;
}

double CrossEntropyLoss::sigmoid(double x) const
{
    if (x >= 0) {
        return 1.0 / (1.0 + qExp(-x));
    } else {
        double ez = qExp(x);
        return ez / (1.0 + ez);
    }
}

double CrossEntropyLoss::accuracy() const
{
    if (m_stats.totalSamples == 0) return 0.0;
    return static_cast<double>(m_stats.totalCorrectPredictions) /
           m_stats.totalSamples;
}

double CrossEntropyLoss::averageLoss() const
{
    if (m_stats.totalSamples == 0) return 0.0;
    return m_stats.cumulativeLoss / m_stats.totalSamples;
}

CrossEntropyLoss::Config CrossEntropyLoss::config() const
{
    return m_config;
}

CrossEntropyLoss::Stats CrossEntropyLoss::stats() const
{
    return m_stats;
}

void CrossEntropyLoss::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

QVector<double> CrossEntropyLoss::applyLabelSmoothing(
    const QVector<double>& target, double smoothing) const
{
    int K = target.size();
    if (K <= 1) return target;

    QVector<double> smoothed(K);
    double uniform = smoothing / K;
    for (int i = 0; i < K; ++i) {
        smoothed[i] = target[i] * (1.0 - smoothing) + uniform;
    }
    return smoothed;
}
