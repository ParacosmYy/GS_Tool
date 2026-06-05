/**
 * @file TripletLoss.h
 * @brief 三元组损失函数,用于度量学习和嵌入训练
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 三元组损失(Triplet Loss)计算器
 *
 * 支持在线/离线三元组挖掘、半硬负例选择、
 * 嵌入距离计算和梯度更新。适用于度量学习场景,
 * 如人脸识别、行人重识别等嵌入空间训练。
 */
class TripletLoss : public QObject
{
    Q_OBJECT

public:
    /** @brief 距离度量类型 */
    enum DistanceMetric {
        Euclidean = 0,  ///< 欧氏距离
        Cosine = 1,     ///< 余弦距离
        Manhattan = 2   ///< 曼哈顿距离
    };
    Q_ENUM(DistanceMetric)

    /** @brief 三元组挖掘策略 */
    enum MiningStrategy {
        All = 0,        ///< 使用所有有效三元组
        Hard = 1,       ///< 硬负例挖掘
        SemiHard = 2    ///< 半硬负例挖掘
    };
    Q_ENUM(MiningStrategy)

    /** @brief 三元组样本 */
    struct Triplet {
        QVector<double> anchor;      ///< 锚点嵌入
        QVector<double> positive;    ///< 正例嵌入
        QVector<double> negative;    ///< 负例嵌入
        double anchorPosDist = 0.0;  ///< 锚-正距离
        double anchorNegDist = 0.0;  ///< 锚-负距离
        double loss = 0.0;           ///< 该三元组损失
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalLossComputed = 0;       ///< 总损失计算次数
        int totalTripletsMined = 0;      ///< 总挖掘三元组数
        double avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    explicit TripletLoss(QObject* parent = nullptr);

    /**
     * @brief 计算单个三元组的损失
     * @param anchor 锚点嵌入向量
     * @param positive 正例嵌入向量
     * @param negative 负例嵌入向量
     * @param margin 间隔参数margin
     * @param metric 距离度量类型
     * @return 损失值(≥0)
     */
    double computeLoss(const QVector<double>& anchor,
                       const QVector<double>& positive,
                       const QVector<double>& negative,
                       double margin = 1.0,
                       DistanceMetric metric = Euclidean);

    /**
     * @brief 批量计算三元组损失
     * @param embeddings 嵌入向量矩阵(每行一个向量)
     * @param labels 对应标签
     * @param margin 间隔参数
     * @param strategy 挖掘策略
     * @param metric 距离度量
     * @return {平均损失, 有效三元组列表}
     */
    QPair<double, QVector<Triplet>> batchLoss(
        const QVector<QVector<double>>& embeddings,
        const QVector<int>& labels,
        double margin = 1.0,
        MiningStrategy strategy = SemiHard,
        DistanceMetric metric = Euclidean);

    /**
     * @brief 在线半硬负例挖掘
     * @param anchorIdx 锚点索引
     * @param embeddings 所有嵌入向量
     * @param labels 对应标签
     * @param anchorPosDist 锚-正距离
     * @param metric 距离度量
     * @return 挖掘到的负例索引, -1表示未找到
     */
    int mineSemiHardNegative(int anchorIdx,
                             const QVector<QVector<double>>& embeddings,
                             const QVector<int>& labels,
                             double anchorPosDist,
                             DistanceMetric metric = Euclidean);

    /**
     * @brief 计算两个嵌入向量之间的距离
     * @param a 向量a
     * @param b 向量b
     * @param metric 距离度量类型
     * @return 距离值
     */
    double distance(const QVector<double>& a,
                    const QVector<double>& b,
                    DistanceMetric metric = Euclidean) const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 批量损失计算完成信号 */
    void batchLossComputed(double meanLoss, int tripletCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    double euclideanDist(const QVector<double>& a,
                         const QVector<double>& b) const;
    double cosineDist(const QVector<double>& a,
                      const QVector<double>& b) const;
    double manhattanDist(const QVector<double>& a,
                         const QVector<double>& b) const;
};
