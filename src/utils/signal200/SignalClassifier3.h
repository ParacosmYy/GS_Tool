/**
 * @file SignalClassifier3.h
 * @brief 信号分类器(小波包特征+随机森林集成投票) — Signal Classifier with Wavelet Packet Features and Random Forest Ensemble Voting
 *
 * 功能: 实现信号分类器，支持小波包特征提取、
 *       随机森林集成投票和多类信号识别。
 *
 * 协作: WaveletTransform5(小波变换) / SignalClassifier2(信号分类) / FftEngine3(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 信号分类器(小波包特征+随机森林)
 */
class SignalClassifier3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalClassifications = 0;
        int numSamples = 0;
        int numFeatures = 0;
        int numClasses = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Decision tree node */
    struct TreeNode {
        int featureIndex = -1;
        double threshold = 0.0;
        int leftChild = -1;
        int rightChild = -1;
        int classLabel = -1;    // leaf: class label; internal: -1
    };

    /** @brief A single decision tree */
    struct DecisionTree {
        QVector<TreeNode> nodes;
        double weight = 1.0;
    };

    explicit SignalClassifier3(QObject *parent = nullptr);
    ~SignalClassifier3() override;

    void setNumTrees(int n);
    void setMaxTreeDepth(int depth);
    void setWaveletDecompositionLevel(int level);
    void setNumClasses(int n);

    /** @brief Train classifier on labeled data */
    void train(const QVector<QVector<double>>& signals, const QVector<int>& labels);

    /** @brief Classify a single signal */
    int classify(const QVector<double>& signal);

    /** @brief Classify with probability estimates */
    QVector<double> classifyProba(const QVector<double>& signal) const;

    /** @brief Extract wavelet packet features from signal */
    QVector<double> extractFeatures(const QVector<double>& signal) const;

    /** @brief Wavelet packet decomposition at given level */
    QVector<QVector<double>> waveletPacketDecomp(const QVector<double>& signal, int level) const;

    /** @brief Compute statistics from a coefficient band */
    QVector<double> bandFeatures(const QVector<double>& coeffs) const;

    /** @brief Predict using a single decision tree */
    int predictTree(const DecisionTree& tree, const QVector<double>& features) const;

    /** @brief Ensemble vote across all trees */
    int ensembleVote(const QVector<double>& features) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void trainingCompleted(int trees, double accuracy, double timeMs);
    void classificationCompleted(int label, double confidence, double timeMs);

private:
    int m_numTrees = 50;
    int m_maxDepth = 10;
    int m_waveletLevel = 4;
    int m_numClasses = 5;

    QVector<DecisionTree> m_forest;
    QVector<double> m_featureMin;    // normalization
    QVector<double> m_featureMax;
    bool m_trained = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build a single decision tree via random feature bagging */
    DecisionTree buildTree(const QVector<QVector<double>>& X,
                           const QVector<int>& y,
                           QVector<int>& indices, int depth) const;

    /** @brief Find best split for a feature */
    QPair<double, double> bestSplit(const QVector<QVector<double>>& X,
                                    const QVector<int>& y,
                                    const QVector<int>& indices, int feature) const;

    /** @brief Compute Gini impurity */
    double giniImpurity(const QVector<int>& y, const QVector<int>& indices) const;

    /** @brief Haar wavelet filter */
    void haarFilter(const QVector<double>& in, QVector<double>& lo, QVector<double>& hi) const;
};
