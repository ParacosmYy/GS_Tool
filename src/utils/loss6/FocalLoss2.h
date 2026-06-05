/**
 * @file FocalLoss2.h
 * @brief Focal Loss损失函数 — 不平衡分类的gamma/alpha调谐实现
 *
 * 功能: 实现Focal Loss (Lin et al., 2017)，通过gamma指数抑制
 *       易分类样本的损失贡献，配合alpha权重平衡正负样本比例。
 *       支持二分类和多分类两种模式，提供梯度计算和阈值推荐。
 *
 * 协作: AnomalyDetector(异常检测) / BayesClassifier(贝叶斯分类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>
#include <QElapsedTimer>

/**
 * @brief Focal Loss损失函数计算器
 */
class FocalLoss2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        int    totalComputations = 0;     ///< 累计计算次数
        int    totalSamples = 0;          ///< 累计处理样本数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 损失计算结果
     */
    struct LossResult {
        double loss = 0.0;              ///< 总损失值
        double perSampleLoss = 0.0;     ///< 样本平均损失
        double gradNorm = 0.0;          ///< 梯度L2范数
        double posRatio = 0.0;          ///< 正样本比例
        double negRatio = 0.0;          ///< 负样本比例
        int    easyCount = 0;           ///< 易分类样本数(p>0.9)
        int    hardCount = 0;           ///< 难分类样本数(p<0.3)
    };

    explicit FocalLoss2(QObject* parent = nullptr);

    /**
     * @brief 计算二分类Focal Loss
     * @param predictions 模型预测概率 [0, 1]
     * @param targets 真实标签 {0, 1}
     * @param gamma 聚焦参数(默认2.0，越大越关注难样本)
     * @param alpha 正样本权重(默认0.25)
     * @return 损失计算结果
     */
    LossResult binaryLoss(const QVector<double>& predictions,
                          const QVector<int>& targets,
                          double gamma = 2.0,
                          double alpha = 0.25);

    /**
     * @brief 计算多分类Focal Loss
     * @param predictions NxK预测概率矩阵(每行和为1)
     * @param targets 真实类别标签 [0, K-1]
     * @param gamma 聚焦参数
     * @param alpha 各类权重(K个值，和为1)
     * @return 损失计算结果
     */
    LossResult multiClassLoss(const QVector<QVector<double>>& predictions,
                              const QVector<int>& targets,
                              double gamma = 2.0,
                              const QVector<double>& alpha = {});

    /**
     * @brief 计算二分类梯度
     * @param pred 预测概率
     * @param target 真实标签
     * @param gamma 聚焦参数
     * @param alpha 正样本权重
     * @return 梯度值
     */
    double gradient(double pred, int target, double gamma = 2.0,
                    double alpha = 0.25) const;

    /**
     * @brief 推荐最优阈值
     * @param predictions 预测概率
     * @param targets 真实标签
     * @param gamma 聚焦参数
     * @return 最优阈值及对应F1分数
     */
    QPair<double, double> recommendThreshold(const QVector<double>& predictions,
                                             const QVector<int>& targets,
                                             double gamma = 2.0);

    /**
     * @brief 设置数值稳定性的epsilon
     * @param eps 裁剪下界(默认1e-7)
     */
    void setEpsilon(double eps) { m_epsilon = eps; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param loss 损失值 @param gradNorm 梯度范数 */
    void computationCompleted(double loss, double gradNorm);

private:
    /** @brief 安全log计算 */
    double safeLog(double x) const;

    /** @brief 计算Focal调制因子 (1-p_t)^gamma */
    double focalModulator(double pt, double gamma) const;

    /** @brief 计算F1分数 */
    double computeF1(const QVector<double>& preds,
                     const QVector<int>& targets,
                     double threshold) const;

    Stats              m_stats;
    double             m_timeSum = 0.0;
    double             m_epsilon = 1e-7; ///< 数值裁剪下界
    mutable QElapsedTimer m_timer;
};
