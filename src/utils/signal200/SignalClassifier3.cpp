/**
 * @file SignalClassifier3.cpp
 * @brief SignalClassifier3 实现
 *
 * 实现信号分类器：小波包特征提取、随机森林集成投票、多类信号识别。
 */

#include "utils/signal200/SignalClassifier3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QRandomGenerator>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SignalClassifier3::SignalClassifier3(QObject *parent) : QObject(parent) {}
SignalClassifier3::~SignalClassifier3() = default;

/* ---- Configuration ---- */

void SignalClassifier3::setNumTrees(int n) { m_numTrees = qMax(1, n); }
void SignalClassifier3::setMaxTreeDepth(int d) { m_maxDepth = qMax(1, d); }
void SignalClassifier3::setWaveletDecompositionLevel(int l) { m_waveletLevel = qBound(1, l, 8); }
void SignalClassifier3::setNumClasses(int n) { m_numClasses = qMax(2, n); }

/* ---- Haar wavelet filter ---- */

void SignalClassifier3::haarFilter(const QVector<double>& in,
                                   QVector<double>& lo, QVector<double>& hi) const
{
    int n = in.size() / 2;
    lo.resize(n);
    hi.resize(n);
    double s = 1.0 / qSqrt(2.0);
    for (int i = 0; i < n; ++i) {
        lo[i] = s * (in[2*i] + in[2*i+1]);
        hi[i] = s * (in[2*i] - in[2*i+1]);
    }
}

/* ---- Wavelet packet decomposition ---- */

QVector<QVector<double>> SignalClassifier3::waveletPacketDecomp(
    const QVector<double>& signal, int level) const
{
    QVector<QVector<double>> bands;

    // Pad signal to power of 2
    int N = signal.size();
    int p2 = 1;
    while (p2 < N) p2 <<= 1;

    QVector<QVector<double>> currentBands;
    QVector<double> padded(p2, 0.0);
    for (int i = 0; i < N; ++i) padded[i] = signal[i];
    currentBands.append(padded);

    for (int l = 0; l < level; ++l) {
        QVector<QVector<double>> nextBands;
        for (const auto& band : currentBands) {
            QVector<double> lo, hi;
            haarFilter(band, lo, hi);
            nextBands.append(lo);
            nextBands.append(hi);
        }
        currentBands = nextBands;
    }

    return currentBands;
}

/* ---- Band features ---- */

QVector<double> SignalClassifier3::bandFeatures(const QVector<double>& coeffs) const
{
    if (coeffs.isEmpty()) return {0.0, 0.0, 0.0, 0.0, 0.0};

    double mean = 0.0;
    for (double c : coeffs) mean += c;
    mean /= coeffs.size();

    double var = 0.0;
    for (double c : coeffs) var += (c - mean) * (c - mean);
    var /= coeffs.size();

    double rms = qSqrt(var + mean * mean);

    double maxC = coeffs[0], minC = coeffs[0];
    for (double c : coeffs) { maxC = qMax(maxC, c); minC = qMin(minC, c); }

    // Spectral flatness approximation
    double logSum = 0.0;
    for (double c : coeffs) logSum += qLn(qMax(qAbs(c), 1e-10));
    double geoMean = qExp(logSum / coeffs.size());

    return {mean, qSqrt(var), rms, maxC - minC, geoMean};
}

/* ---- Extract features ---- */

QVector<double> SignalClassifier3::extractFeatures(const QVector<double>& signal) const
{
    auto bands = waveletPacketDecomp(signal, m_waveletLevel);

    QVector<double> features;
    features.append(signal.size() > 0 ? signal[0] : 0.0);  // DC offset

    for (const auto& band : bands) {
        auto bf = bandFeatures(band);
        for (double f : bf) features.append(f);
    }

    // Global features
    double energy = 0.0;
    for (double s : signal) energy += s * s;
    features.append(qSqrt(energy / signal.size()));

    // Zero crossing rate
    int zc = 0;
    for (int i = 1; i < signal.size(); ++i)
        if ((signal[i] >= 0) != (signal[i-1] >= 0)) zc++;
    features.append(static_cast<double>(zc) / signal.size());

    return features;
}

/* ---- Gini impurity ---- */

double SignalClassifier3::giniImpurity(const QVector<int>& y,
                                       const QVector<int>& indices) const
{
    if (indices.isEmpty()) return 0.0;
    QVector<int> counts(m_numClasses, 0);
    for (int idx : indices)
        if (y[idx] >= 0 && y[idx] < m_numClasses) counts[y[idx]]++;
    double gini = 1.0;
    double n = indices.size();
    for (int c : counts) {
        double p = c / n;
        gini -= p * p;
    }
    return gini;
}

/* ---- Best split ---- */

QPair<double, double> SignalClassifier3::bestSplit(
    const QVector<QVector<double>>& X, const QVector<int>& y,
    const QVector<int>& indices, int feature) const
{
    // Sort by feature value
    QVector<QPair<double, int>> sorted;
    for (int idx : indices)
        sorted.append({X[idx][feature], idx});
    std::sort(sorted.begin(), sorted.end());

    double bestThreshold = sorted[0].first;
    double bestGini = std::numeric_limits<double>::max();

    for (int i = 1; i < sorted.size(); ++i) {
        if (sorted[i].first == sorted[i-1].first) continue;
        double threshold = (sorted[i].first + sorted[i-1].first) / 2.0;

        QVector<int> left, right;
        for (int j = 0; j < i; ++j) left.append(sorted[j].second);
        for (int j = i; j < sorted.size(); ++j) right.append(sorted[j].second);

        double g = (left.size() * giniImpurity(y, left) +
                   right.size() * giniImpurity(y, right)) / sorted.size();
        if (g < bestGini) { bestGini = g; bestThreshold = threshold; }
    }
    return {bestThreshold, bestGini};
}

/* ---- Build tree ---- */

SignalClassifier3::DecisionTree SignalClassifier3::buildTree(
    const QVector<QVector<double>>& X, const QVector<int>& y,
    QVector<int>& indices, int depth) const
{
    DecisionTree tree;
    if (indices.isEmpty() || depth >= m_maxDepth) {
        TreeNode leaf;
        // Majority class
        QVector<int> counts(m_numClasses, 0);
        for (int idx : indices)
            if (y[idx] >= 0 && y[idx] < m_numClasses) counts[y[idx]]++;
        leaf.classLabel = 0;
        for (int c = 1; c < m_numClasses; ++c)
            if (counts[c] > counts[leaf.classLabel]) leaf.classLabel = c;
        tree.nodes.append(leaf);
        return tree;
    }

    // Check pure node
    int firstLabel = y[indices[0]];
    bool pure = true;
    for (int idx : indices) if (y[idx] != firstLabel) { pure = false; break; }
    if (pure) {
        TreeNode leaf; leaf.classLabel = firstLabel;
        tree.nodes.append(leaf);
        return tree;
    }

    // Random feature selection
    int numFeat = X[0].size();
    int numTry = qMax(1, static_cast<int>(qSqrt(numFeat)));
    QVector<int> featIndices(numFeat);
    for (int i = 0; i < numFeat; ++i) featIndices[i] = i;
    std::shuffle(featIndices.begin(), featIndices.end(), *QRandomGenerator::global());
    featIndices.resize(numTry);

    double bestGini = std::numeric_limits<double>::max();
    int bestFeat = -1;
    double bestThresh = 0.0;
    for (int f : featIndices) {
        auto [thresh, gini] = bestSplit(X, y, indices, f);
        if (gini < bestGini) { bestGini = gini; bestFeat = f; bestThresh = thresh; }
    }

    TreeNode node;
    node.featureIndex = bestFeat;
    node.threshold = bestThresh;
    node.classLabel = -1;
    int nodeIdx = tree.nodes.size();
    tree.nodes.append(node);

    // Split
    QVector<int> leftIdx, rightIdx;
    for (int idx : indices) {
        if (X[idx][bestFeat] <= bestThresh) leftIdx.append(idx);
        else rightIdx.append(idx);
    }

    auto leftTree = buildTree(X, y, leftIdx, depth + 1);
    int leftRoot = tree.nodes.size();
    for (auto& n : leftTree.nodes) {
        if (n.leftChild >= 0) n.leftChild += leftRoot;
        if (n.rightChild >= 0) n.rightChild += leftRoot;
    }
    tree.nodes.append(leftTree.nodes);

    auto rightTree = buildTree(X, y, rightIdx, depth + 1);
    int rightRoot = tree.nodes.size();
    for (auto& n : rightTree.nodes) {
        if (n.leftChild >= 0) n.leftChild += rightRoot;
        if (n.rightChild >= 0) n.rightChild += rightRoot;
    }
    tree.nodes.append(rightTree.nodes);

    tree.nodes[nodeIdx].leftChild = leftRoot;
    tree.nodes[nodeIdx].rightChild = rightRoot;
    return tree;
}

/* ---- Predict single tree ---- */

int SignalClassifier3::predictTree(const DecisionTree& tree,
                                   const QVector<double>& features) const
{
    int idx = 0;
    while (idx >= 0 && idx < tree.nodes.size()) {
        const auto& node = tree.nodes[idx];
        if (node.classLabel >= 0) return node.classLabel;
        if (node.featureIndex < 0 || node.featureIndex >= features.size()) break;
        idx = (features[node.featureIndex] <= node.threshold) ? node.leftChild : node.rightChild;
    }
    return 0;
}

/* ---- Ensemble vote ---- */

int SignalClassifier3::ensembleVote(const QVector<double>& features) const
{
    QVector<double> votes(m_numClasses, 0.0);
    for (const auto& tree : m_forest)
        votes[predictTree(tree, features)] += tree.weight;
    int best = 0;
    for (int c = 1; c < m_numClasses; ++c)
        if (votes[c] > votes[best]) best = c;
    return best;
}

/* ---- Train ---- */

void SignalClassifier3::train(const QVector<QVector<double>>& signals,
                              const QVector<int>& labels)
{
    QElapsedTimer timer;
    timer.start();

    int n = signals.size();
    if (n == 0 || labels.size() != n) return;

    // Extract features for all signals
    QVector<QVector<double>> X(n);
    for (int i = 0; i < n; ++i)
        X[i] = extractFeatures(signals[i]);

    // Compute normalization bounds
    int numFeat = X[0].size();
    m_featureMin.resize(numFeat);
    m_featureMax.resize(numFeat);
    for (int f = 0; f < numFeat; ++f) {
        m_featureMin[f] = std::numeric_limits<double>::max();
        m_featureMax[f] = -std::numeric_limits<double>::max();
        for (int i = 0; i < n; ++i) {
            m_featureMin[f] = qMin(m_featureMin[f], X[i][f]);
            m_featureMax[f] = qMax(m_featureMax[f], X[i][f]);
        }
    }

    // Normalize
    for (int i = 0; i < n; ++i)
        for (int f = 0; f < numFeat; ++f) {
            double range = m_featureMax[f] - m_featureMin[f];
            X[i][f] = (range > 1e-10) ? (X[i][f] - m_featureMin[f]) / range : 0.0;
        }

    // Build random forest with bagging
    m_forest.resize(m_numTrees);
    QVector<int> allIndices(n);
    for (int i = 0; i < n; ++i) allIndices[i] = i;

    for (int t = 0; t < m_numTrees; ++t) {
        // Bootstrap sample
        QVector<int> sample;
        for (int i = 0; i < n; ++i)
            sample.append(allIndices[QRandomGenerator::global()->bounded(n)]);
        m_forest[t] = buildTree(X, labels, sample, 0);
        m_forest[t].weight = 1.0;
    }

    // Compute training accuracy
    int correct = 0;
    for (int i = 0; i < n; ++i)
        if (ensembleVote(X[i]) == labels[i]) correct++;
    double accuracy = static_cast<double>(correct) / n;

    m_trained = true;
    m_stats.numSamples = n;
    m_stats.numFeatures = numFeat;
    m_stats.numClasses = m_numClasses;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum;

    emit trainingCompleted(m_numTrees, accuracy, timer.elapsed());
}

/* ---- Classify ---- */

int SignalClassifier3::classify(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    auto features = extractFeatures(signal);

    // Normalize
    for (int f = 0; f < features.size() && f < m_featureMin.size(); ++f) {
        double range = m_featureMax[f] - m_featureMin[f];
        features[f] = (range > 1e-10) ? (features[f] - m_featureMin[f]) / range : 0.0;
    }

    int label = ensembleVote(features);

    m_stats.totalClassifications++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClassifications;

    emit classificationCompleted(label, 1.0, timer.elapsed());
    return label;
}

/* ---- Classify with probabilities ---- */

QVector<double> SignalClassifier3::classifyProba(const QVector<double>& signal) const
{
    if (!m_trained) return QVector<double>(m_numClasses, 1.0 / m_numClasses);

    auto features = extractFeatures(signal);
    for (int f = 0; f < features.size() && f < m_featureMin.size(); ++f) {
        double range = m_featureMax[f] - m_featureMin[f];
        features[f] = (range > 1e-10) ? (features[f] - m_featureMin[f]) / range : 0.0;
    }

    QVector<double> votes(m_numClasses, 0.0);
    for (const auto& tree : m_forest)
        votes[predictTree(tree, features)] += tree.weight;

    double total = 0.0;
    for (double v : votes) total += v;
    if (total > 0.0)
        for (double& v : votes) v /= total;
    return votes;
}

/* ---- Reset ---- */

void SignalClassifier3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_forest.clear();
    m_featureMin.clear();
    m_featureMax.clear();
    m_trained = false;
}
