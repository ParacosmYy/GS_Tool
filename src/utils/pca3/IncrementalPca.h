/**
 * @file IncrementalPca.h
 * @brief 增量PCA — 在线/流式主成分分析
 *
 * 功能: 实现增量式PCA(IPCA)，支持逐样本或小批量更新，
 *       无需存储全部数据即可维护主成分。适用于流数据
 *       和大数据集的降维。
 *
 * 协作: DataTransformer(数据变换) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <vector>

/**
 * @brief 增量PCA — 在线/流式主成分分析
 */
class IncrementalPca : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSamples         = 0;   ///< 累计处理样本数
        quint64 totalComponents       = 0;   ///< 累计主成分更新次数
        double  avgProcessingTimeMs   = 0.0; ///< 平均处理时间(ms)
        double  explainedVariance     = 0.0; ///< 已解释方差比
        int     currentComponents     = 0;   ///< 当前主成分数
    };

    /**
     * @brief 构造函数
     * @param dimensions 数据维度
     * @param numComponents 目标主成分数(0=自动)
     * @param parent 父对象
     */
    explicit IncrementalPca(int dimensions = 2, int numComponents = 0,
                            QObject* parent = nullptr);

    /**
     * @brief 添加单个样本
     * @param sample 样本向量(维度必须匹配)
     */
    void addSample(const QVector<double>& sample);

    /**
     * @brief 添加批量样本
     * @param samples 样本列表
     */
    void addBatch(const QVector<QVector<double>>& samples);

    /**
     * @brief 将样本投影到主成分空间
     * @param sample 原始样本
     * @return 降维后的向量
     */
    QVector<double> transform(const QVector<double>& sample) const;

    /**
     * @brief 从主成分空间重建样本
     * @param projected 降维向量
     * @return 重建后的原始空间向量
     */
    QVector<double> inverseTransform(const QVector<double>& projected) const;

    /**
     * @brief 获取主成分(特征向量)
     * @param index 主成分索引(0-based)
     * @return 特征向量
     */
    QVector<double> component(int index) const;

    /**
     * @brief 获取特征值
     * @return 特征值列表(降序)
     */
    QVector<double> explainedVariances() const;

    /**
     * @brief 获取已解释方差比
     * @return 每个主成分的方差百分比
     */
    QVector<double> explainedVarianceRatios() const;

    /** @brief 数据维度 @return 维度 */
    int dimensions() const { return m_dims; }

    /** @brief 主成分数 @return 成分数 */
    int numComponents() const { return m_numComp; }

    /** @brief 已处理样本数 @return 样本数 */
    int sampleCount() const { return static_cast<int>(m_sampleCount); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 主成分更新完成 @param components 成分数 */
    void componentsUpdated(int components);

    /** @brief 批量处理完成 @param count 样本数 */
    void batchProcessed(int count);

private:
    /**
     * @brief CCIPCA算法核心更新
     * @param sample 新样本
     */
    void updateCCIPCA(const std::vector<double>& sample);

    /**
     * @brief 对给定主成分应用修正Gram-Schmidt正交化
     * @param vec 待正交化的向量
     * @param compIdx 当前成分索引
     */
    void orthogonalize(std::vector<double>& vec, int compIdx);

    int m_dims;        ///< 数据维度
    int m_numComp;     ///< 目标主成分数
    int m_sampleCount; ///< 样本计数

    std::vector<double>                m_mean;   ///< 运行均值
    std::vector<std::vector<double>>   m_eigenVectors; ///< 特征向量
    std::vector<double>                m_eigenValues;  ///< 特征值
    double m_totalVariance;                          ///< 总方差

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;
};
