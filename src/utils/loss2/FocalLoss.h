/**
 * @file FocalLoss.h
 * @brief Focal Loss损失函数(Focal Loss)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class FocalLoss
 * @brief Focal Loss — 解决类别不平衡的损失函数
 *
 * 支持二分类/多分类Focal Loss、alpha/gamma参数调节。
 * 适用于不平衡数据集训练、难样本挖掘等场景。
 */
class FocalLoss : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalComputed = 0;    /**< 总计算次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit FocalLoss(double gamma = 2.0, double alpha = 0.25,
                         QObject* parent = nullptr);

    /**
     * @brief 二分类Focal Loss
     * @param prediction 预测概率
     * @param target 真实标签(0或1)
     * @return 损失值
     */
    double binaryLoss(double prediction, int target) const;

    /**
     * @brief 二分类Focal Loss(批量)
     * @param predictions 预测概率列表
     * @param targets 真实标签列表
     * @return 平均损失值
     */
    double binaryLossBatch(const QVector<double>& predictions,
                             const QVector<int>& targets) const;

    /**
     * @brief 多分类Focal Loss
     * @param classProbabilities 各类概率
     * @param targetClass 正确类别索引
     * @return 损失值
     */
    double multiClassLoss(const QVector<double>& classProbabilities,
                            int targetClass) const;

    /**
     * @brief 多分类Focal Loss(批量)
     * @param batchProbs 批量概率(每行一个样本)
     * @param targets 类别索引列表
     * @return 平均损失值
     */
    double multiClassLossBatch(const QVector<QVector<double>>& batchProbs,
                                 const QVector<int>& targets) const;

    /**
     * @brief 计算梯度(二分类)
     * @param prediction 预测概率
     * @param target 真实标签
     * @return 梯度值
     */
    double gradient(double prediction, int target) const;

    /** @brief 设置gamma参数 */
    void setGamma(double gamma);

    /** @brief 设置alpha参数 */
    void setAlpha(double alpha);

    double gamma() const;
    double alpha() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void computed(int batchSize, double loss);

private:
    double m_gamma;
    double m_alpha;
    Stats m_stats;
    double m_timeSum;
};
