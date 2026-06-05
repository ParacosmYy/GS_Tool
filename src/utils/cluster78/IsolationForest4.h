#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief IsolationForest4 - 孤立森林异常检测器
 *
 * 基于随机森林的异常检测算法，通过随机特征选择
 * 和分割点构建孤立树，异常点路径长度更短。
 */
class IsolationForest4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTreesBuilt = 0;
        int totalAnomaliesDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IsolationForest4(QObject* parent = nullptr);

    /** @brief 设置树的数量和子采样大小 */
    void setParameters(int numTrees, int sampleSize = 256);

    /** @brief 训练孤立森林模型 */
    void fit(const QVector<QVector<double>>& data);

    /** @brief 计算样本的异常分数(0~1, 越高越异常) */
    double anomalyScore(const QVector<double>& sample) const;

    /** @brief 批量预测，返回异常样本索引 */
    QVector<int> detectAnomalies(const QVector<QVector<double>>& data,
                                  double threshold = 0.6) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void forestBuilt(int treeCount, int depthLimit);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_numTrees = 100;
    int m_sampleSize = 256;
    int m_maxDepth = 0;
};
