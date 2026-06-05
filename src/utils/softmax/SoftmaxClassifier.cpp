/**
 * @file SoftmaxClassifier.cpp
 * @brief Softmax分类器实现
 */

#include "utils/softmax/SoftmaxClassifier.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

SoftmaxClassifier::SoftmaxClassifier(QObject* parent)
    : QObject(parent)
{
}

SoftmaxClassifier::TrainResult SoftmaxClassifier::train(
    const QVector<QVector<double>>& X,
    const QVector<int>& labels,
    int nClasses,
    const TrainConfig& config)
{
    QElapsedTimer timer;
    timer.start();
    TrainResult result;

    int nSamples = X.size();
    if (nSamples == 0 || labels.size() != nSamples) return result;

    m_nClasses = nClasses;
    m_nFeatures = X[0].size();

    /* 初始化权重和偏置(小随机值) */
    std::mt19937 gen(42);
    std::normal_distribution<double> dist(0.0, 0.01);

    m_weights.resize(nClasses);
    for (int k = 0; k < nClasses; ++k) {
        m_weights[k].resize(m_nFeatures);
        for (int j = 0; j < m_nFeatures; ++j) {
            m_weights[k][j] = dist(gen);
        }
    }
    m_biases.resize(nClasses, 0.0);

    int batchSize = (config.batchSize <= 0) ? nSamples :
                    qMin(config.batchSize, nSamples);

    /* 梯度下降训练 */
    double prevLoss = 1e18;
    for (int epoch = 0; epoch < config.maxEpochs; ++epoch) {
        /* 生成样本索引(可选打乱) */
        QVector<int> indices(nSamples);
        std::iota(indices.begin(), indices.end(), 0);
        if (config.shuffle) {
            std::shuffle(indices.begin(), indices.end(), gen);
        }

        /* 小批量梯度下降 */
        for (int batchStart = 0; batchStart < nSamples;
             batchStart += batchSize) {
            int bEnd = qMin(batchStart + batchSize, nSamples);
            int bSize = bEnd - batchStart;

            /* 计算梯度 */
            QVector<QVector<double>> gradW(nClasses,
                QVector<double>(m_nFeatures, 0.0));
            QVector<double> gradB(nClasses, 0.0);

            for (int bi = batchStart; bi < bEnd; ++bi) {
                int i = indices[bi];
                const auto& features = X[i];
                int label = labels[i];

                /* 计算logits */
                QVector<double> logits(nClasses, 0.0);
                for (int k = 0; k < nClasses; ++k) {
                    double sum = m_biases[k];
                    for (int j = 0; j < m_nFeatures; ++j) {
                        sum += m_weights[k][j] * features[j];
                    }
                    logits[k] = sum;
                }

                /* Softmax概率 */
                QVector<double> probs = softmax(logits);

                /* 梯度: dL/dw_k = (p_k - y_k) * x */
                for (int k = 0; k < nClasses; ++k) {
                    double diff = probs[k] - ((k == label) ? 1.0 : 0.0);
                    for (int j = 0; j < m_nFeatures; ++j) {
                        gradW[k][j] += diff * features[j];
                    }
                    gradB[k] += diff;
                }
            }

            /* 更新参数 */
            double scale = config.learningRate / bSize;
            for (int k = 0; k < nClasses; ++k) {
                for (int j = 0; j < m_nFeatures; ++j) {
                    m_weights[k][j] -= scale * gradW[k][j];
                    /* L2正则化 */
                    if (config.lambda > 0.0) {
                        m_weights[k][j] -= config.learningRate
                            * config.lambda * m_weights[k][j];
                    }
                }
                m_biases[k] -= scale * gradB[k];
            }

            m_stats.totalTrainSteps++;
        }

        /* 计算本轮损失 */
        double loss = crossEntropyLoss(X, labels);
        result.lossHistory.append(loss);

        /* 检查收敛 */
        if (qAbs(prevLoss - loss) < config.tolerance) {
            result.finalLoss = loss;
            result.epochsCompleted = epoch + 1;
            break;
        }
        prevLoss = loss;
    }

    result.epochsCompleted = (result.epochsCompleted > 0) ?
        result.epochsCompleted : config.maxEpochs;
    result.finalLoss = result.lossHistory.isEmpty() ? 0.0 :
        result.lossHistory.last();
    result.trainAccuracy = accuracy(X, labels);

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalPredictions + m_stats.totalTrainSteps > 0)
        ? m_timeSum / (m_stats.totalPredictions + m_stats.totalTrainSteps) : 0.0;

    emit trainingCompleted(result.epochsCompleted, result.finalLoss);
    return result;
}

int SoftmaxClassifier::predict(const QVector<double>& features) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> logits(m_nClasses, 0.0);
    for (int k = 0; k < m_nClasses; ++k) {
        double sum = m_biases[k];
        for (int j = 0; j < m_nFeatures && j < features.size(); ++j) {
            sum += m_weights[k][j] * features[j];
        }
        logits[k] = sum;
    }

    int bestClass = 0;
    double bestLogit = logits[0];
    for (int k = 1; k < m_nClasses; ++k) {
        if (logits[k] > bestLogit) {
            bestLogit = logits[k];
            bestClass = k;
        }
    }

    m_stats.totalPredictions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalPredictions + m_stats.totalTrainSteps > 0)
        ? m_timeSum / (m_stats.totalPredictions + m_stats.totalTrainSteps) : 0.0;

    emit predictionCompleted(bestClass);
    return bestClass;
}

QVector<double> SoftmaxClassifier::predictProba(
    const QVector<double>& features) const
{
    QVector<double> logits(m_nClasses, 0.0);
    for (int k = 0; k < m_nClasses; ++k) {
        double sum = m_biases[k];
        for (int j = 0; j < m_nFeatures && j < features.size(); ++j) {
            sum += m_weights[k][j] * features[j];
        }
        logits[k] = sum;
    }
    return softmax(logits);
}

QVector<int> SoftmaxClassifier::predictBatch(
    const QVector<QVector<double>>& X) const
{
    QVector<int> predictions;
    predictions.reserve(X.size());
    for (const auto& sample : X) {
        QVector<double> logits(m_nClasses, 0.0);
        for (int k = 0; k < m_nClasses; ++k) {
            double sum = m_biases[k];
            for (int j = 0; j < m_nFeatures && j < sample.size(); ++j) {
                sum += m_weights[k][j] * sample[j];
            }
            logits[k] = sum;
        }
        int best = 0;
        for (int k = 1; k < m_nClasses; ++k) {
            if (logits[k] > logits[best]) best = k;
        }
        predictions.append(best);
    }
    return predictions;
}

double SoftmaxClassifier::accuracy(
    const QVector<QVector<double>>& X,
    const QVector<int>& labels) const
{
    if (X.size() != labels.size() || X.isEmpty()) return 0.0;
    int correct = 0;
    auto preds = predictBatch(X);
    for (int i = 0; i < preds.size(); ++i) {
        if (preds[i] == labels[i]) ++correct;
    }
    return static_cast<double>(correct) / preds.size();
}

QVector<QVector<double>> SoftmaxClassifier::weights() const
{
    return m_weights;
}

QVector<double> SoftmaxClassifier::biases() const
{
    return m_biases;
}

QVector<double> SoftmaxClassifier::softmax(
    const QVector<double>& logits) const
{
    int n = logits.size();
    if (n == 0) return {};

    /* 数值稳定: 减去最大值 */
    double maxVal = *std::max_element(logits.begin(), logits.end());
    QVector<double> exps(n);
    double sumExps = 0.0;
    for (int i = 0; i < n; ++i) {
        exps[i] = qExp(logits[i] - maxVal);
        sumExps += exps[i];
    }
    QVector<double> probs(n);
    for (int i = 0; i < n; ++i) {
        probs[i] = exps[i] / sumExps;
    }
    return probs;
}

double SoftmaxClassifier::crossEntropyLoss(
    const QVector<QVector<double>>& X,
    const QVector<int>& labels) const
{
    int n = X.size();
    double totalLoss = 0.0;
    for (int i = 0; i < n; ++i) {
        QVector<double> logits(m_nClasses, 0.0);
        for (int k = 0; k < m_nClasses; ++k) {
            double sum = m_biases[k];
            for (int j = 0; j < m_nFeatures && j < X[i].size(); ++j) {
                sum += m_weights[k][j] * X[i][j];
            }
            logits[k] = sum;
        }
        QVector<double> probs = softmax(logits);
        int label = labels[i];
        double p = qBound(1e-15, probs[label], 1.0 - 1e-15);
        totalLoss -= qLog(p);
    }
    return totalLoss / n;
}

SoftmaxClassifier::Stats SoftmaxClassifier::stats() const
{
    return m_stats;
}

void SoftmaxClassifier::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
