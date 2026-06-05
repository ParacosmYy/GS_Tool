/**
 * @file GaussianNaiveBayes.h
 * @brief 高斯朴素贝叶斯分类器 — Laplace平滑
 *
 * 功能: 实现高斯朴素贝叶斯(GNB)分类器, 假设各特征在给定类别下独立且服从高斯分布,
 *       支持Laplace平滑防止零概率, 提供训练、预测、交叉验证和特征重要性分析。
 *       GNB是嵌入式系统中轻量级分类的理想选择。
 *
 * 协作: AnomalyDetector(异常检测) / FeatureExtractor(特征提取) / SensorClassifier(传感器分类)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 高斯朴素贝叶斯分类器
 *
 * 训练阶段: 对每个类别估计每个特征的均值和方差(最大似然)。
 * 预测阶段: 使用贝叶斯定理 P(y|x) ∝ P(y) ∏ P(xi|y)。
 * Laplace平滑: 在方差估计中加入平滑参数, 防止退化。
 */
class GaussianNaiveBayes : public QObject
{
    Q_OBJECT

public:
    /** @brief 分类结果 */
    struct Prediction {
        int predictedClass = 0;             ///< 预测类别
        double confidence = 0.0;            ///< 置信度(后验概率)
        QMap<int, double> classProbabilities; ///< 各类别后验概率
        double logPosterior = 0.0;          ///< 对数后验概率
    };

    /** @brief 交叉验证结果 */
    struct CrossValidationResult {
        double accuracy = 0.0;              ///< 准确率
        double precision = 0.0;             ///< 宏平均精确率
        double recall = 0.0;                ///< 宏平均召回率
        double f1Score = 0.0;               ///< F1分数
        QMap<int, double> perClassAccuracy; ///< 各类准确率
        QMap<int, int> confusionRow;        ///< 混淆矩阵行
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSamplesTrained = 0;        ///< 累计训练样本数
        int totalPredictions = 0;           ///< 累计预测次数
        int totalCrossValidations = 0;      ///< 累计交叉验证次数
        int totalFeatures = 0;              ///< 当前特征维度
        int totalClasses = 0;               ///< 当前类别数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit GaussianNaiveBayes(QObject* parent = nullptr);

    /**
     * @brief 训练分类器
     * @param features 特征矩阵 [样本数][特征数]
     * @param labels 类别标签 [样本数]
     * @param laplaceSmoothing Laplace平滑参数(加到方差上, 默认1e-9)
     * @return true=训练成功
     */
    bool train(const QVector<QVector<double>>& features,
               const QVector<int>& labels,
               double laplaceSmoothing = 1e-9);

    /**
     * @brief 预测单个样本的类别
     * @param features 样本特征向量
     * @return 分类结果(含后验概率)
     */
    Prediction predict(const QVector<double>& features) const;

    /**
     * @brief 批量预测
     * @param features 特征矩阵
     * @return 分类结果数组
     */
    QVector<Prediction> predictBatch(const QVector<QVector<double>>& features) const;

    /**
     * @brief K折交叉验证
     * @param features 特征矩阵
     * @param labels 标签数组
     * @param k 折数(默认5)
     * @return 交叉验证结果
     */
    CrossValidationResult crossValidate(const QVector<QVector<double>>& features,
                                         const QVector<int>& labels,
                                         int k = 5) const;

    /**
     * @brief 计算特征重要性(基于类间方差比)
     * @return 特征重要性得分 [0~1], 越大越重要
     */
    QVector<double> featureImportance() const;

    /**
     * @brief 获取各类别的参数摘要
     * @return QMap: key=类别, value={meanVector, varianceVector, prior}
     */
    QMap<int, QPair<QVector<double>, QVector<double>>> classParameters() const;

    /**
     * @brief 计算训练集准确率
     * @param features 特征矩阵
     * @param labels 真实标签
     * @return 准确率 [0~1]
     */
    double accuracy(const QVector<QVector<double>>& features,
                     const QVector<int>& labels) const;

    /** @brief 获取类别列表 */
    QVector<int> classes() const;

    /** @brief 获取类别先验概率 */
    QMap<int, double> classPriors() const;

    Stats stats() const;
    void resetStatistics();

private:
    /**
     * @brief 计算高斯概率密度函数值
     * @param x 特征值
     * @param mean 均值
     * @param variance 方差
     * @return 概率密度
     */
    double gaussianPDF(double x, double mean, double variance) const;

    /**
     * @brief 计算对数高斯PDF(避免下溢)
     * @param x 特征值
     * @param mean 均值
     * @param variance 方差
     * @return 对数概率密度
     */
    double logGaussianPDF(double x, double mean, double variance) const;

    /**
     * @brief 随机打乱索引数组(用于交叉验证)
     * @param n 数组长度
     * @return 打乱后的索引
     */
    QVector<int> shuffleIndices(int n) const;

    QMap<int, QVector<double>> m_classMeans;    ///< 各类各特征均值
    QMap<int, QVector<double>> m_classVariances; ///< 各类各特征方差
    QMap<int, double> m_classPriors;             ///< 各类先验概率
    QVector<int> m_classes;                      ///< 类别列表(排序)
    int m_numFeatures = 0;                       ///< 特征维度

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;
};
