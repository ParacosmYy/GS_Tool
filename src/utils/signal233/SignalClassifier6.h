/**
 * @file SignalClassifier6.h
 * @brief 信号分类器(小波包能量特征+随机森林OOB误差) — Signal Classifier with Wavelet Packet Energy Features and Random Forest Ensemble with Out-of-Bag Error Estimation
 *
 * 功能: 实现信号分类器，使用小波包分解提取能量特征(wavelet packet energy features)，
 *       通过随机森林集成(random forest ensemble)进行分类，并利用OOB(out-of-bag)误差评估泛化性能。
 *
 * 协作: WaveletTransform5(小波变换) / PeakDetector7(峰值检测) / EnvelopeDetector6(包络检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号分类器(小波包能量特征+随机森林OOB误差)
 */
class SignalClassifier6 : public QObject {
    Q_OBJECT

public:
    /** @brief Decision tree node in the random forest */
    struct TreeNode {
        int featureIndex = -1;
        double threshold = 0.0;
        int leftChild = -1;
        int rightChild = -1;
        int classLabel = -1;   // leaf: class label; internal: -1
    };

    /** @brief Single decision tree */
    struct DecisionTree {
        QVector<TreeNode> nodes;
        QVector<int> oobIndices;
        double oobAccuracy = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numFeatures = 0;
        int numClasses = 0;
        int numSamples = 0;
        int numTrees = 0;
        double oobError = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SignalClassifier6(QObject *parent = nullptr);
    ~SignalClassifier6() override;

    /** @brief Set classifier parameters */
    void setParameters(int numTrees, int maxDepth, int numWaveletLevels, int sampleRate = 1000);

    /** @brief Train classifier with labeled signals */
    bool train(const QVector<QVector<double>>& signals, const QVector<int>& labels);

    /** @brief Predict class for a single signal */
    int predict(const QVector<double>& signal) const;

    /** @brief Predict class probabilities for a signal */
    QVector<double> predictProba(const QVector<double>& signal) const;

    /** @brief Extract wavelet packet energy features from a signal */
    QVector<double> extractFeatures(const QVector<double>& signal) const;

    /** @brief Get OOB error estimate */
    double oobError() const;

    /** @brief Get feature importance scores */
    QVector<double> featureImportance() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void trainCompleted(int numTrees, double oobError, double timeMs);
    void treeTrained(int treeIndex, double treeOobAccuracy);

private:
    int m_numTrees = 100;
    int m_maxDepth = 10;
    int m_numWaveletLevels = 4;
    int m_sampleRate = 1000;
    int m_numClasses = 0;
    int m_numFeatures = 0;

    QVector<DecisionTree> m_forest;
    QVector<double> m_featureImportance;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Wavelet packet decomposition for feature extraction */
    QVector<QVector<double>> waveletPacketDecompose(const QVector<double>& signal) const;

    /** @brief Haar wavelet filter bank (one level) */
    void haarFilter(const QVector<double>& input,
                    QVector<double>& approx, QVector<double>& detail) const;

    /** @brief Compute energy of each wavelet packet sub-band */
    QVector<double> computeSubbandEnergies(const QVector<QVector<double>>& subbands) const;

    /** @brief Build a single decision tree with bootstrap sample */
    DecisionTree buildTree(const QVector<QVector<double>>& features,
                            const QVector<int>& labels,
                            const QVector<int>& sampleIndices) const;

    /** @brief Find best split for a set of samples */
    void findBestSplit(const QVector<QVector<double>>& features,
                        const QVector<int>& labels,
                        const QVector<int>& indices,
                        int& bestFeature, double& bestThreshold) const;

    /** @brief Compute Gini impurity */
    double giniImpurity(const QVector<int>& labels, const QVector<int>& indices) const;

    /** @brief Traverse a single tree for prediction */
    int traverseTree(const DecisionTree& tree, const QVector<double>& features) const;

    /** @brief Compute OOB error for a trained tree */
    double computeTreeOob(const DecisionTree& tree,
                           const QVector<QVector<double>>& features,
                           const QVector<int>& labels) const;
};
