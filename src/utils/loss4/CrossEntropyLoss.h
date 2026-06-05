/**
 * @file CrossEntropyLoss.h
 * @brief 交叉熵损失函数 — 含Softmax集成
 *
 * 功能: 计算分类任务的交叉熵损失, 支持二分类和多分类,
 *       内置数值稳定的Softmax, 提供损失曲线和梯度计算。
 *
 * 协作: DataClassifier(分类器) / DataQualityScorer(质量评估)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 交叉熵损失计算器
 *
 * 支持的损失类型:
 * - 二元交叉熵 (BCE): sigmoid + 二分类
 * - 分类交叉熵 (CCE): softmax + 多分类
 * - 稀疏交叉熵: 整数标签直接输入
 * - 标签平滑交叉熵: 防止过拟合
 */
class CrossEntropyLoss : public QObject
{
    Q_OBJECT

public:
    /** @brief 损失函数类型 */
    enum class LossType {
        BinaryCrossEntropy,         ///< 二元交叉熵
        CategoricalCrossEntropy,    ///< 分类交叉熵(softmax)
        SparseCategoricalCE,        ///< 稀疏分类交叉熵
        LabelSmoothedCE             ///< 标签平滑交叉熵
    };
    Q_ENUM(LossType)

    /** @brief 损失计算配置 */
    struct Config {
        LossType type = LossType::CategoricalCrossEntropy;
        double labelSmoothing = 0.1;        ///< 标签平滑系数(0=无平滑)
        double epsilon = 1e-7;              ///< 数值稳定小量
        bool computeGradient = false;       ///< 是否计算梯度
    };

    /** @brief 损失计算结果 */
    struct LossResult {
        double loss = 0.0;                  ///< 损失值
        double perplexity = 0.0;            ///< 困惑度
        int predictedClass = -1;            ///< 预测类别
        double confidence = 0.0;            ///< 预测置信度
        QVector<double> probabilities;      ///< Softmax概率分布
        QVector<double> gradient;           ///< 损失对logits的梯度
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalLossComputations = 0;      ///< 累计损失计算次数
        int totalSamples = 0;               ///< 累计样本数
        int totalCorrectPredictions = 0;    ///< 累计正确预测数
        double cumulativeLoss = 0.0;        ///< 累计损失
        double minLoss = 1e18;              ///< 最小损失
        double maxLoss = 0.0;               ///< 最大损失
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit CrossEntropyLoss(QObject* parent = nullptr);

    /**
     * @brief 配置损失函数
     * @param config 配置参数
     */
    void configure(const Config& config);

    /**
     * @brief 计算分类交叉熵损失(Softmax + CE)
     * @param logits 未归一化对数概率
     * @param classIndex 正确类别索引(0-based)
     * @return 损失计算结果
     */
    LossResult compute(const QVector<double>& logits, int classIndex);

    /**
     * @brief 计算多标签交叉熵损失
     * @param logits 未归一化对数概率
     * @param targetProbs 目标概率分布
     * @return 损失计算结果
     */
    LossResult computeMultiTarget(const QVector<double>& logits,
                                  const QVector<double>& targetProbs);

    /**
     * @brief 批量计算损失
     * @param batchLogits 批次logits(每个元素是一组logits)
     * @param labels 对应的类别索引
     * @return 平均损失值
     */
    double computeBatch(const QVector<QVector<double>>& batchLogits,
                        const QVector<int>& labels);

    /**
     * @brief 数值稳定的Softmax
     * @param logits 输入logits
     * @return 概率分布
     */
    QVector<double> softmax(const QVector<double>& logits) const;

    /**
     * @brief Sigmoid函数
     * @param x 输入值
     * @return sigmoid(x)
     */
    double sigmoid(double x) const;

    /**
     * @brief 计算准确率
     * @return 当前准确率(0~1)
     */
    double accuracy() const;

    /**
     * @brief 计算平均损失
     * @return 当前平均损失
     */
    double averageLoss() const;

    Config config() const;
    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 损失计算完成 @param loss 损失值 @param perplexity 困惑度 */
    void lossComputed(double loss, double perplexity);

    /** @brief 批次处理完成 @param avgLoss 平均损失 @param accuracy 准确率 */
    void batchCompleted(double avgLoss, double accuracy);

private:
    /**
     * @brief 应用标签平滑
     * @param target 原始目标(独热)
     * @param smoothing 平滑系数
     * @return 平滑后的目标分布
     */
    QVector<double> applyLabelSmoothing(const QVector<double>& target,
                                        double smoothing) const;

    Config m_config;
    Stats m_stats;
    double m_timeSum = 0.0;
};
