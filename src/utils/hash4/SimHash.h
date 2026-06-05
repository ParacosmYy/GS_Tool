/**
 * @file SimHash.h
 * @brief SimHash局部敏感哈希
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QByteArray>

/**
 * @class SimHash
 * @brief SimHash — 局部敏感哈希，用于近似重复检测
 *
 * 将高维特征向量映射为固定位数的指纹，相似文档有相似的指纹。
 * 支持汉明距离阈值判断相似度。
 */
class SimHash : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalComputed = 0;      /**< 总计算次数 */
        int totalComparisons = 0;   /**< 总比较次数 */
        int totalSimilar = 0;       /**< 相似次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param hashBits 哈希位数(默认64)
     * @param parent 父对象
     */
    explicit SimHash(int hashBits = 64, QObject* parent = nullptr);

    /**
     * @brief 从特征集计算SimHash
     * @param features 特征列表[(特征, 权重), ...]
     * @return SimHash指纹值
     */
    quint64 compute(const QVector<QPair<QString, double>>& features) const;

    /**
     * @brief 从文本计算SimHash(基于n-gram)
     * @param text 文本
     * @param ngramSize n-gram大小(默认3)
     * @return SimHash指纹值
     */
    quint64 fromText(const QString& text, int ngramSize = 3) const;

    /**
     * @brief 计算两个指纹的汉明距离
     * @param h1 第一个指纹
     * @param h2 第二个指纹
     * @return 汉明距离
     */
    int hammingDistance(quint64 h1, quint64 h2) const;

    /**
     * @brief 判断两个指纹是否相似
     * @param h1 第一个指纹
     * @param h2 第二个指纹
     * @param threshold 距离阈值(默认3)
     * @return 是否相似
     */
    bool isSimilar(quint64 h1, quint64 h2, int threshold = 3) const;

    /**
     * @brief 计算相似度(0~1)
     * @param h1 第一个指纹
     * @param h2 第二个指纹
     * @return 相似度
     */
    double similarity(quint64 h1, quint64 h2) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void hashComputed(quint64 hashValue);

private:
    quint64 hashFeature(const QString& feature) const;

    int m_hashBits;
    mutable Stats m_stats;
    mutable double m_timeSum;
};
