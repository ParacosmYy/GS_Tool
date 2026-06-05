/**
 * @file SoftmaxClassifier.h
 * @brief Softmax分类器 — 多分类逻辑回归
 *
 * 支持多类别分类, 使用交叉熵损失函数和梯度下降/小批量SGD优化。
 * 包含训练、预测、概率估计、精度评估等完整功能。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class SoftmaxClassifier
 * @brief Softmax多分类器 — 交叉熵损失 + 梯度下降
 *
 * 模型: P(y=k|x) = exp(w_k^T x) / sum_j exp(w_j^T x)
 * 损失: L = -sum_k y_k * log(P(y=k|x))  (交叉熵)
 */
class SoftmaxClassifier : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalPredictions = 0;   ///< 总预测次数
        quint64 totalTrainSteps = 0;    ///< 总训练步数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 训练参数 */
    struct TrainConfig {
        double learningRate = 0.01;  ///< 学习率
        int maxEpochs = 100;         ///< 最大训练轮数
        int batchSize = 32;          ///< 批量大小(0=全批量)
        double tolerance = 1e-6;     ///< 收敛容差
        double lambda = 0.0;         ///< L2正则化系数
        bool shuffle = true;         ///< 是否每轮打乱数据
    };

    /** @brief 训练结果 */
    struct TrainResult {
        int epochsCompleted = 0;     ///< 完成的训练轮数
        double finalLoss = 0.0;      ///< 最终损失值
        QVector<double> lossHistory; ///< 每轮损失记录
        double trainAccuracy = 0.0;  ///< 训练集精度
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit SoftmaxClassifier(QObject* parent = nullptr);

    /**
     * @brief 训练分类器
     * @param X 特征矩阵 [nSamples x nFeatures]
     * @param labels 标签向量 [nSamples], 值为0~nClasses-1
     * @param nClasses 类别数
     * @param config 训练参数
     * @return 训练结果
     */
    TrainResult train(const QVector<QVector<double>>& X,
                      const QVector<int>& labels,
                      int nClasses,
                      const TrainConfig& config = TrainConfig());

    /**
     * @brief 预测单个样本的类别
     * @param features 特征向量
     * @return 预测类别(0~nClasses-1)
     */
    int predict(const QVector<double>& features) const;

    /**
     * @brief 预测概率分布
     * @param features 特征向量
     * @return 各类别概率 [nClasses]
     */
    QVector<double> predictProba(const QVector<double>& features) const;

    /**
     * @brief 批量预测
     * @param X 特征矩阵
     * @return 预测标签列表
     */
    QVector<int> predictBatch(const QVector<QVector<double>>& X) const;

    /**
     * @brief 计算精度
     * @param X 特征矩阵
     * @param labels 真实标签
     * @return 精度 [0,1]
     */
    double accuracy(const QVector<QVector<double>>& X,
                    const QVector<int>& labels) const;

    /** @brief 获取权重矩阵 [nClasses x nFeatures] */
    QVector<QVector<double>> weights() const;

    /** @brief 获取偏置 [nClasses] */
    QVector<double> biases() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 训练完成 @param epochs 轮数 @param loss 最终损失 */
    void trainingCompleted(int epochs, double loss);
    /** @brief 预测完成 @param predictedClass 预测类别 */
    void predictionCompleted(int predictedClass);

private:
    /** @brief 计算Softmax概率 */
    QVector<double> softmax(const QVector<double>& logits) const;

    /** @brief 计算交叉熵损失 */
    double crossEntropyLoss(const QVector<QVector<double>>& X,
                            const QVector<int>& labels) const;

    QVector<QVector<double>> m_weights; ///< 权重矩阵 [nClasses x nFeatures]
    QVector<double> m_biases;           ///< 偏置 [nClasses]
    int m_nClasses = 0;                 ///< 类别数
    int m_nFeatures = 0;                ///< 特征数

    mutable Stats m_stats;     ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
