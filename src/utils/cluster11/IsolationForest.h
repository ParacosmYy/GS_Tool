/**
 * @file IsolationForest.h
 * @brief 孤立森林算法 — 基于随机隔离的无监督异常检测
 *
 * 功能: 通过随机选择特征和分割点构建多棵隔离树，利用样本
 *       到根节点的路径长度衡量异常程度。路径越短越异常。
 *       支持多维特征、异常分数归一化、批量预测。
 *
 * 协作: AnomalyDetector(异常检测框架) / DataClassifier(分类标注)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QRandomGenerator>

/**
 * @brief 孤立森林异常检测引擎 — 随机隔离路径长度评分
 */
class IsolationForest : public QObject {
    Q_OBJECT

public:
    /** @brief 异常检测结果 */
    struct AnomalyResult {
        double score = 0.0;            ///< 异常分数[0,1]，越接近1越异常
        int pathLength = 0;            ///< 平均路径长度
        bool isAnomaly = false;        ///< 是否判定为异常
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        int totalTreesBuilt = 0;               ///< 累计构建树数
        int totalSamplesTrained = 0;           ///< 累计训练样本数
        int totalPredictions = 0;              ///< 累计预测次数
        int totalAnomaliesFound = 0;           ///< 累计发现异常数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit IsolationForest(QObject* parent = nullptr);

    /** @brief 设置树的数量 @param count 树数量 */
    void setTreeCount(int count);

    /** @brief 设置每棵树的子采样大小 @param size 子采样大小 */
    void setSampleSize(int size);

    /** @brief 设置最大树深度 @param depth 最大深度 */
    void setMaxDepth(int depth);

    /** @brief 设置异常判定阈值 @param threshold 阈值[0,1] */
    void setAnomalyThreshold(double threshold);

    /** @brief 训练孤立森林 @param data 训练数据[样本 x 特征] */
    void fit(const QVector<QVector<double>>& data);

    /** @brief 预测单个样本 @param sample 样本特征向量 @return 异常检测结果 */
    AnomalyResult predict(const QVector<double>& sample) const;

    /** @brief 批量预测 @param data 样本集合 @return 结果列表 */
    QList<AnomalyResult> predictBatch(
        const QVector<QVector<double>>& data) const;

    /** @brief 获取当前统计 @return 统计常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 训练完成 @param trees 树数 @param samples 训练样本数 */
    void trainingComplete(int trees, int samples);

    /** @brief 检测到异常 @param score 异常分数 */
    void anomalyDetected(double score);

private:
    /**
     * @brief 隔离树节点
     */
    struct IsoNode {
        int splitFeature = -1;         ///< 分割特征索引
        double splitValue = 0.0;       ///< 分割阈值
        int left = -1;                 ///< 左子节点索引(-1为叶)
        int right = -1;                ///< 右子节点索引(-1为叶)
        int depth = 0;                 ///< 节点深度
        int size = 0;                  ///< 叶节点包含的样本数
    };

    /** @brief 构建单棵隔离树 @param data 训练数据 @param indices 样本索引 @param depth 当前深度 @param rng 随机数生成器 @return 树节点列表 */
    QVector<IsoNode> buildTree(
        const QVector<QVector<double>>& data,
        const QVector<int>& indices, int depth,
        QRandomGenerator& rng);

    /** @brief 计算样本在树中的路径长度 @param sample 样本 @param tree 树 @return 路径长度 */
    int pathLength(const QVector<double>& sample,
                   const QVector<IsoNode>& tree) const;

    /** @brief 计算平均路径长度期望值(用于归一化) @param n 样本数 @return 期望路径长度 */
    static double averagePathLength(int n);

    int m_treeCount;                   ///< 树数量
    int m_sampleSize;                  ///< 子采样大小
    int m_maxDepth;                    ///< 最大树深度
    double m_threshold;                ///< 异常阈值

    QVector<QVector<IsoNode>> m_forest;///< 孤立森林(多棵树)
    int m_featureCount;                ///< 特征维度
    int m_trainSize;                   ///< 训练集大小

    Stats m_stats;                     ///< 运行时统计
    double m_timeSum = 0.0;            ///< 累计耗时(ms)
};
