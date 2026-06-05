#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class IsolationForest3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit IsolationForest3(QObject* parent = nullptr);
    void setNumTrees(int n);
    void setSampleSize(int size);
    void setMaxDepth(int depth);
    void setContamination(double c);
    QVector<double> fit(const QVector<QVector<double>>& data);
    QVector<int> predict(const QVector<QVector<double>>& data);
    int numTrees() const { return m_numTrees; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void fitCompleted(int trees, double threshold);
private:
    int m_numTrees = 100; int m_sampleSize = 256; int m_maxDepth = 0; double m_contamination = 0.1;
    double m_threshold = 0.0;
    struct ITreeNode;
    ITreeNode* buildTree(const QVector<QVector<double>>& data, int depth);
    double pathLength(ITreeNode* node, const QVector<double>& point, int depth);
    double cFactor(int n) const;
    Stats m_stats; double m_timeSum = 0.0;
};
