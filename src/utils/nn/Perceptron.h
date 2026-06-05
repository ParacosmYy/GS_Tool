/**
 * @file Perceptron.h
 * @brief 感知机(Perceptron)线性分类器
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class Perceptron
 * @brief 感知机 — 最简单的线性二分类器
 *
 * 支持批量训练和在线预测。适用于线性可分数据的快速分类。
 */
class Perceptron : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalTrained = 0;       /**< 总训练次数 */
        int totalPredictions = 0;   /**< 总预测次数 */
        int totalEpochs = 0;        /**< 总训练轮数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param numFeatures 特征维度
     * @param learningRate 学习率(默认0.1)
     * @param maxEpochs 最大训练轮数(默认1000)
     * @param parent 父对象
     */
    explicit Perceptron(int numFeatures, double learningRate = 0.1,
                         int maxEpochs = 1000, QObject* parent = nullptr);

    /**
     * @brief 训练
     * @param data 训练数据[(特征向量, 标签±1), ...]
     * @return 是否收敛
     */
    bool train(const QVector<QPair<QVector<double>, int>>& data);

    /**
     * @brief 预测
     * @param features 特征向量
     * @return 预测标签(+1或-1)
     */
    int predict(const QVector<double>& features) const;

    /**
     * @brief 预测置信度
     * @param features 特征向量
     * @return 原始输出值(正=正类, 负=负类)
     */
    double confidence(const QVector<double>& features) const;

    /** @brief 获取权重 */
    QVector<double> weights() const;

    /** @brief 获取偏置 */
    double bias() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 训练完成信号 */
    void trainCompleted(int epochs, bool converged);

private:
    QVector<double> m_weights;     /**< 权重向量 */
    double m_bias;                 /**< 偏置 */
    double m_lr;                   /**< 学习率 */
    int m_maxEpochs;               /**< 最大训练轮数 */

    Stats m_stats;
    double m_timeSum;
};
