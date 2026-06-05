/**
 * @file BaggingEnsemble.cpp
 * @brief Bootstrap聚合集成学习实现
 */

#include "utils/ensemble/BaggingEnsemble.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>
#include <algorithm>
#include <random>
#include <map>

/** @brief 构造函数 @param parent 父对象 */
BaggingEnsemble::BaggingEnsemble(QObject* parent)
    : QObject(parent)
{
}

/** @brief 训练Bagging集成 */
double BaggingEnsemble::train(const QVector<QVector<double>>& data,
                               const QVector<int>& labels,
                               int nModels)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0 || labels.size() != n || nModels <= 0) return 0.0;

    m_trainData = data;
    m_trainLabels = labels;
    m_nFeatures = (n > 0) ? data[0].size() : 0;
    m_predictions.clear();
    m_bootstrapIndices.clear();
    m_oobIndices.clear();

    std::random_device rd;
    std::mt19937 gen(rd());

    int oobCorrect = 0;
    int oobTotal = 0;

    for (int m = 0; m < nModels; ++m) {
        /* 生成Bootstrap样本 */
        QVector<int> bootIdx = bootstrapSample(n);
        m_bootstrapIndices.append(bootIdx);

        /* 计算OOB索引 */
        QSet<int> bootSet(bootIdx.begin(), bootIdx.end());
        QVector<int> oobIdx;
        for (int i = 0; i < n; ++i) {
            if (!bootSet.contains(i)) {
                oobIdx.append(i);
            }
        }
        m_oobIndices.append(oobIdx);

        /* 构建Bootstrap训练集 */
        QVector<QVector<double>> bootData;
        QVector<int> bootLabels;
        for (int idx : bootIdx) {
            bootData.append(data[idx]);
            bootLabels.append(labels[idx]);
        }

        /* 训练基模型: 使用简单的最近质心分类器作为基模型 */
        /* 统计各类别各特征的均值(质心) */
        QMap<int, QVector<double>> centroids;
        QMap<int, int> classCounts;

        for (int i = 0; i < bootData.size(); ++i) {
            int label = bootLabels[i];
            classCounts[label]++;
            if (!centroids.contains(label)) {
                centroids[label] = QVector<double>(m_nFeatures, 0.0);
            }
            for (int f = 0; f < m_nFeatures; ++f) {
                centroids[label][f] += bootData[i][f];
            }
        }

        for (auto it = centroids.begin(); it != centroids.end(); ++it) {
            int cnt = classCounts[it.key()];
            for (int f = 0; f < m_nFeatures; ++f) {
                it.value()[f] /= cnt;
            }
        }

        /* 创建预测函数(捕获质心) */
        auto capturedCentroids = centroids;
        PredictFn predFn = [capturedCentroids](const QVector<double>& sample) -> int {
            double bestDist = 1e18;
            int bestLabel = 0;
            for (auto it = capturedCentroids.constBegin();
                 it != capturedCentroids.constEnd(); ++it) {
                double dist = 0.0;
                for (int i = 0; i < qMin(sample.size(), it.value().size()); ++i) {
                    double diff = sample[i] - it.value()[i];
                    dist += diff * diff;
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    bestLabel = it.key();
                }
            }
            return bestLabel;
        };

        m_predictions.append(predFn);

        /* OOB误差估计 */
        for (int idx : oobIdx) {
            int pred = predFn(data[idx]);
            oobTotal++;
            if (pred == labels[idx]) oobCorrect++;
        }
    }

    double oobAccuracy = (oobTotal > 0)
        ? static_cast<double>(oobCorrect) / oobTotal : 0.0;

    m_stats.totalTrainings++;
    m_timeSum += timer.elapsed();
    double total = static_cast<double>(m_stats.totalTrainings +
                                        m_stats.totalPredictions);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit trainingCompleted(nModels);
    return oobAccuracy;
}

/** @brief 多数投票预测 */
int BaggingEnsemble::predict(const QVector<double>& sample)
{
    m_stats.totalPredictions++;

    if (m_predictions.isEmpty()) return 0;

    /* 投票计数 */
    QMap<int, int> votes;
    for (const auto& predFn : m_predictions) {
        int pred = predFn(sample);
        votes[pred]++;
    }

    /* 选择票数最多的类别 */
    int bestLabel = 0;
    int bestVotes = 0;
    for (auto it = votes.constBegin(); it != votes.constEnd(); ++it) {
        if (it.value() > bestVotes) {
            bestVotes = it.value();
            bestLabel = it.key();
        }
    }
    return bestLabel;
}

/** @brief 计算特征重要性(置换法) */
QVector<double> BaggingEnsemble::featureImportance() const
{
    if (m_trainData.isEmpty() || m_predictions.isEmpty()) {
        return QVector<double>(m_nFeatures, 0.0);
    }

    int n = m_trainData.size();
    QVector<double> importance(m_nFeatures, 0.0);

    /* 计算OOB基线准确率 */
    int correct = 0;
    int total = 0;
    for (int m = 0; m < m_predictions.size(); ++m) {
        for (int idx : m_oobIndices[m]) {
            total++;
            if (m_predictions[m](m_trainData[idx]) == m_trainLabels[idx]) {
                correct++;
            }
        }
    }
    double baseline = (total > 0) ? static_cast<double>(correct) / total : 0.0;

    /* 对每个特征置换后计算准确率下降 */
    for (int f = 0; f < m_nFeatures; ++f) {
        int permCorrect = 0;
        int permTotal = 0;

        for (int m = 0; m < m_predictions.size(); ++m) {
            for (int idx : m_oobIndices[m]) {
                /* 置换第f个特征 */
                QVector<double> permSample = m_trainData[idx];
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dist(0, n - 1);
                permSample[f] = m_trainData[dist(gen)][f];

                permTotal++;
                if (m_predictions[m](permSample) == m_trainLabels[idx]) {
                    permCorrect++;
                }
            }
        }

        double permAcc = (permTotal > 0)
            ? static_cast<double>(permCorrect) / permTotal : 0.0;
        importance[f] = baseline - permAcc;
    }

    return importance;
}

/** @brief 重置统计 */
void BaggingEnsemble::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 生成Bootstrap样本索引 */
QVector<int> BaggingEnsemble::bootstrapSample(int n) const
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, n - 1);

    QVector<int> sample(n);
    for (int i = 0; i < n; ++i) {
        sample[i] = dist(gen);
    }
    return sample;
}
