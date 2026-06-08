/**
 * @file SignalClassifier6.cpp
 * @brief SignalClassifier6 实现
 *
 * 实现信号分类器：小波包能量特征与随机森林OOB误差估计。
 */

#include "utils/signal233/SignalClassifier6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

SignalClassifier6::SignalClassifier6(QObject *parent) : QObject(parent) {}
SignalClassifier6::~SignalClassifier6() = default;

/* ---- Configuration ---- */

void SignalClassifier6::setParameters(int numTrees, int maxDepth, int numWaveletLevels, int sampleRate)
{
    m_numTrees = qMax(1, numTrees);
    m_maxDepth = qMax(1, maxDepth);
    m_numWaveletLevels = qMax(1, numWaveletLevels);
    m_sampleRate = qMax(1, sampleRate);
}

/* ---- Haar filter bank ---- */

void SignalClassifier6::haarFilter(const QVector<double>& input,
                                      QVector<double>& approx, QVector<double>& detail) const
{
    int n = input.size() / 2;
    approx.resize(n);
    detail.resize(n);

    for (int i = 0; i < n; ++i) {
        approx[i] = (input[2 * i] + input[2 * i + 1]) / qSqrt(2.0);
        detail[i] = (input[2 * i] - input[2 * i + 1]) / qSqrt(2.0);
    }
}

/* ---- Wavelet packet decomposition ---- */

QVector<QVector<double>> SignalClassifier6::waveletPacketDecompose(const QVector<double>& signal) const
{
    QVector<QVector<double>> subbands;

    QVector<QVector<double>> currentLevel;
    currentLevel.append(signal);

    for (int level = 0; level < m_numWaveletLevels; ++level) {
        QVector<QVector<double>> nextLevel;
        for (const auto& band : currentLevel) {
            if (band.size() < 2) {
                nextLevel.append(band);
                continue;
            }
            QVector<double> approx, detail;
            haarFilter(band, approx, detail);
            nextLevel.append(approx);
            nextLevel.append(detail);
        }
        currentLevel = nextLevel;
    }

    return currentLevel;
}

/* ---- Compute subband energies ---- */

QVector<double> SignalClassifier6::computeSubbandEnergies(const QVector<QVector<double>>& subbands) const
{
    QVector<double> energies;
    for (const auto& band : subbands) {
        double energy = 0.0;
        for (double s : band) energy += s * s;
        energies.append(qSqrt(energy / qMax(1, band.size())));
    }

    // Add relative energies and spectral centroid features
    double totalEnergy = 0.0;
    for (double e : energies) totalEnergy += e;
    if (totalEnergy > 0.0)
        for (double& e : energies) e /= totalEnergy;

    return energies;
}

/* ---- Extract features ---- */

QVector<double> SignalClassifier6::extractFeatures(const QVector<double>& signal) const
{
    QVector<QVector<double>> subbands = waveletPacketDecompose(signal);
    QVector<double> features = computeSubbandEnergies(subbands);

    // Add statistical features: mean energy, variance, zero-crossing rate
    double meanVal = 0.0;
    for (double s : signal) meanVal += s;
    meanVal /= signal.size();

    double variance = 0.0;
    for (double s : signal) variance += (s - meanVal) * (s - meanVal);
    variance /= signal.size();

    int zeroCrossings = 0;
    for (int i = 1; i < signal.size(); ++i)
        if ((signal[i] >= 0) != (signal[i - 1] >= 0)) zeroCrossings++;

    features.append(meanVal);
    features.append(qSqrt(variance));
    features.append(static_cast<double>(zeroCrossings) / signal.size());

    return features;
}

/* ---- Gini impurity ---- */

double SignalClassifier6::giniImpurity(const QVector<int>& labels, const QVector<int>& indices) const
{
    if (indices.isEmpty()) return 0.0;

    QVector<int> counts(m_numClasses, 0);
    for (int idx : indices) counts[labels[idx]]++;

    double gini = 1.0;
    double n = indices.size();
    for (int c = 0; c < m_numClasses; ++c) {
        double p = counts[c] / n;
        gini -= p * p;
    }
    return gini;
}

/* ---- Find best split ---- */

void SignalClassifier6::findBestSplit(const QVector<QVector<double>>& features,
                                         const QVector<int>& labels,
                                         const QVector<int>& indices,
                                         int& bestFeature, double& bestThreshold) const
{
    double bestGini = std::numeric_limits<double>::max();
    bestFeature = -1;
    bestThreshold = 0.0;

    int numFeatures = features.isEmpty() ? 0 : features[0].size();
    int numFeaturesToTry = qMax(1, qSqrt(numFeatures));

    // Random feature selection (sqrt(m) for classification)
    QVector<int> featureCandidates;
    for (int f = 0; f < numFeatures; ++f) featureCandidates.append(f);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(featureCandidates.begin(), featureCandidates.end(), g);
    featureCandidates.resize(qMin(numFeaturesToTry, featureCandidates.size()));

    for (int f : featureCandidates) {
        // Get unique thresholds
        QVector<double> thresholds;
        for (int idx : indices) thresholds.append(features[idx][f]);
        std::sort(thresholds.begin(), thresholds.end());
        thresholds.erase(std::unique(thresholds.begin(), thresholds.end()), thresholds.end());

        // Try midpoints between consecutive values
        for (int t = 0; t < qMin(20, thresholds.size() - 1); ++t) {
            double thresh = (thresholds[t] + thresholds[t + 1]) / 2.0;

            QVector<int> leftIdx, rightIdx;
            for (int idx : indices) {
                if (features[idx][f] <= thresh)
                    leftIdx.append(idx);
                else
                    rightIdx.append(idx);
            }

            if (leftIdx.isEmpty() || rightIdx.isEmpty()) continue;

            double leftGini = giniImpurity(labels, leftIdx);
            double rightGini = giniImpurity(labels, rightIdx);
            double weightedGini = (leftIdx.size() * leftGini + rightIdx.size() * rightGini)
                                   / indices.size();

            if (weightedGini < bestGini) {
                bestGini = weightedGini;
                bestFeature = f;
                bestThreshold = thresh;
            }
        }
    }
}

/* ---- Build single decision tree ---- */

SignalClassifier6::DecisionTree SignalClassifier6::buildTree(const QVector<QVector<double>>& features,
                                                                const QVector<int>& labels,
                                                                const QVector<int>& sampleIndices) const
{
    DecisionTree tree;
    int nodeId = 0;

    // Stack-based tree building
    struct StackEntry {
        QVector<int> indices;
        int depth;
        int parentNodeId;
        bool isLeft;
    };

    QVector<StackEntry> stack;
    stack.append({sampleIndices, 0, -1, false});

    while (!stack.isEmpty()) {
        StackEntry entry = stack.takeLast();
        const QVector<int>& indices = entry.indices;

        // Check stopping criteria
        bool pure = true;
        int classLabel = labels[indices[0]];
        for (int idx : indices) {
            if (labels[idx] != classLabel) { pure = false; break; }
        }

        if (pure || entry.depth >= m_maxDepth || indices.size() <= 2) {
            // Leaf node: majority vote
            TreeNode node;
            node.classLabel = 0;
            QVector<int> counts(m_numClasses, 0);
            for (int idx : indices) counts[labels[idx]]++;
            int maxCount = 0;
            for (int c = 0; c < m_numClasses; ++c) {
                if (counts[c] > maxCount) { maxCount = counts[c]; node.classLabel = c; }
            }

            int currentId = tree.nodes.size();
            tree.nodes.append(node);

            if (entry.parentNodeId >= 0) {
                if (entry.isLeft)
                    tree.nodes[entry.parentNodeId].leftChild = currentId;
                else
                    tree.nodes[entry.parentNodeId].rightChild = currentId;
            }
            continue;
        }

        // Find best split
        int bestFeature;
        double bestThreshold;
        findBestSplit(features, labels, indices, bestFeature, bestThreshold);

        TreeNode node;
        node.featureIndex = bestFeature;
        node.threshold = bestThreshold;

        int currentId = tree.nodes.size();
        tree.nodes.append(node);

        if (entry.parentNodeId >= 0) {
            if (entry.isLeft)
                tree.nodes[entry.parentNodeId].leftChild = currentId;
            else
                tree.nodes[entry.parentNodeId].rightChild = currentId;
        }

        // Split indices
        QVector<int> leftIdx, rightIdx;
        for (int idx : indices) {
            if (bestFeature >= 0 && features[idx][bestFeature] <= bestThreshold)
                leftIdx.append(idx);
            else
                rightIdx.append(idx);
        }

        if (leftIdx.isEmpty() || rightIdx.isEmpty()) {
            tree.nodes[currentId].featureIndex = -1;
            tree.nodes[currentId].classLabel = 0;
            QVector<int> counts(m_numClasses, 0);
            for (int idx : indices) counts[labels[idx]]++;
            int maxCount = 0;
            for (int c = 0; c < m_numClasses; ++c) {
                if (counts[c] > maxCount) { maxCount = counts[c]; tree.nodes[currentId].classLabel = c; }
            }
            continue;
        }

        // Reserve space for children (will be filled when popped)
        tree.nodes[currentId].leftChild = -1;
        tree.nodes[currentId].rightChild = -1;

        stack.append({rightIdx, entry.depth + 1, currentId, false});
        stack.append({leftIdx, entry.depth + 1, currentId, true});
    }

    return tree;
}

/* ---- Compute tree OOB ---- */

double SignalClassifier6::computeTreeOob(const DecisionTree& tree,
                                           const QVector<QVector<double>>& features,
                                           const QVector<int>& labels) const
{
    if (tree.oobIndices.isEmpty()) return 0.0;

    int correct = 0;
    for (int idx : tree.oobIndices) {
        if (traverseTree(tree, features[idx]) == labels[idx]) correct++;
    }
    return static_cast<double>(correct) / tree.oobIndices.size();
}

/* ---- Traverse tree ---- */

int SignalClassifier6::traverseTree(const DecisionTree& tree, const QVector<double>& features) const
{
    int nodeId = 0;
    while (nodeId >= 0 && nodeId < tree.nodes.size()) {
        const TreeNode& node = tree.nodes[nodeId];
        if (node.classLabel >= 0 && node.featureIndex < 0) return node.classLabel;
        if (node.featureIndex < 0 || node.featureIndex >= features.size()) return 0;

        if (features[node.featureIndex] <= node.threshold)
            nodeId = node.leftChild;
        else
            nodeId = node.rightChild;
    }
    return 0;
}

/* ---- Train ---- */

bool SignalClassifier6::train(const QVector<QVector<double>>& signals, const QVector<int>& labels)
{
    QElapsedTimer timer;
    timer.start();

    int n = signals.size();
    if (n == 0 || n != labels.size()) return false;

    // Determine number of classes
    int maxClass = 0;
    for (int l : labels) maxClass = qMax(maxClass, l);
    m_numClasses = maxClass + 1;

    // Extract features for all samples
    QVector<QVector<double>> allFeatures(n);
    for (int i = 0; i < n; ++i)
        allFeatures[i] = extractFeatures(signals[i]);

    m_numFeatures = allFeatures.isEmpty() ? 0 : allFeatures[0].size();
    m_forest.clear();
    m_forest.reserve(m_numTrees);

    std::random_device rd;
    std::mt19937 rng(rd());

    // Bootstrap sampling indices
    QVector<int> allIndices(n);
    for (int i = 0; i < n; ++i) allIndices[i] = i;

    for (int t = 0; t < m_numTrees; ++t) {
        // Bootstrap sample
        QVector<int> bootstrap;
        QSet<int> inBag;
        std::uniform_int_distribution<int> dist(0, n - 1);
        for (int i = 0; i < n; ++i) {
            int idx = dist(rng);
            bootstrap.append(idx);
            inBag.insert(idx);
        }

        // OOB indices
        QVector<int> oobIdx;
        for (int i = 0; i < n; ++i)
            if (!inBag.contains(i)) oobIdx.append(i);

        DecisionTree tree = buildTree(allFeatures, labels, bootstrap);
        tree.oobIndices = oobIdx;
        tree.oobAccuracy = computeTreeOob(tree, allFeatures, labels);

        m_forest.append(tree);
        emit treeTrained(t, tree.oobAccuracy);
    }

    // Compute overall OOB error
    int oobCorrect = 0;
    int oobTotal = 0;
    for (int i = 0; i < n; ++i) {
        QVector<int> votes(m_numClasses, 0);
        int numVoters = 0;
        for (const auto& tree : m_forest) {
            if (tree.oobIndices.contains(i)) {
                votes[traverseTree(tree, allFeatures[i])]++;
                numVoters++;
            }
        }
        if (numVoters > 0) {
            int bestClass = 0;
            for (int c = 1; c < m_numClasses; ++c)
                if (votes[c] > votes[bestClass]) bestClass = c;
            if (bestClass == labels[i]) oobCorrect++;
            oobTotal++;
        }
    }
    m_stats.oobError = (oobTotal > 0) ? 1.0 - static_cast<double>(oobCorrect) / oobTotal : 1.0;

    // Compute feature importance
    m_featureImportance = QVector<double>(m_numFeatures, 0.0);
    for (const auto& tree : m_forest) {
        for (const auto& node : tree.nodes) {
            if (node.featureIndex >= 0 && node.featureIndex < m_numFeatures)
                m_featureImportance[node.featureIndex] += 1.0;
        }
    }
    double maxImp = *std::max_element(m_featureImportance.begin(), m_featureImportance.end());
    if (maxImp > 0.0)
        for (double& imp : m_featureImportance) imp /= maxImp;

    m_stats.numFeatures = m_numFeatures;
    m_stats.numClasses = m_numClasses;
    m_stats.numSamples = n;
    m_stats.numTrees = m_numTrees;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit trainCompleted(m_numTrees, m_stats.oobError, timer.elapsed());
    return true;
}

/* ---- Predict ---- */

int SignalClassifier6::predict(const QVector<double>& signal) const
{
    QVector<double> proba = predictProba(signal);
    int best = 0;
    for (int c = 1; c < proba.size(); ++c)
        if (proba[c] > proba[best]) best = c;
    return best;
}

/* ---- Predict probabilities ---- */

QVector<double> SignalClassifier6::predictProba(const QVector<double>& signal) const
{
    QVector<double> features = extractFeatures(signal);
    QVector<double> votes(m_numClasses, 0.0);

    for (const auto& tree : m_forest) {
        int cls = traverseTree(tree, features);
        if (cls >= 0 && cls < m_numClasses) votes[cls] += 1.0;
    }

    double total = 0.0;
    for (double v : votes) total += v;
    if (total > 0.0)
        for (double& v : votes) v /= total;
    return votes;
}

/* ---- Accessors ---- */

double SignalClassifier6::oobError() const { return m_stats.oobError; }

QVector<double> SignalClassifier6::featureImportance() const { return m_featureImportance; }

/* ---- Reset ---- */

void SignalClassifier6::resetStatistics()
{
    m_forest.clear();
    m_featureImportance.clear();
    m_numClasses = 0;
    m_numFeatures = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
