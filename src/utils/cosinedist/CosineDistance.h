/**
 * @file CosineDistance.h
 * @brief 余弦距离/相似度计算器 — 向量相似度度量
 *
 * 功能: 计算两个向量的余弦相似度与余弦距离，
 *       支持批量计算与归一化相似度，统计计算次数与平均耗时。
 */
#ifndef COSINEDISTANCE_H
#define COSINEDISTANCE_H

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 余弦距离/相似度计算器
 */
class CosineDistance : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations = 0;    ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(毫秒)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit CosineDistance(QObject* parent = nullptr);

    /**
     * @brief 计算余弦相似度
     * @param a 向量A
     * @param b 向量B
     * @return 相似度(-1.0~1.0)，零向量返回0.0
     */
    double similarity(const QVector<double>& a, const QVector<double>& b);

    /**
     * @brief 计算余弦距离(1 - similarity)
     * @param a 向量A
     * @param b 向量B
     * @return 距离(0.0~2.0)
     */
    double distance(const QVector<double>& a, const QVector<double>& b);

    /**
     * @brief 批量计算相似度(多个向量与同一目标的相似度)
     * @param vectors 向量集合
     * @param target 目标向量
     * @return 各向量与目标的相似度列表
     */
    QVector<double> batchSimilarity(const QVector<QVector<double>>& vectors,
                                    const QVector<double>& target);

    /**
     * @brief 归一化余弦相似度(映射到[0,1])
     * @param a 向量A
     * @param b 向量B
     * @return 归一化相似度(0.0~1.0)
     */
    double normalizedSimilarity(const QVector<double>& a,
                                const QVector<double>& b);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 计算完成 @param result 计算结果 */
    void computationCompleted(double result);

private:
    /**
     * @brief 计算点积
     * @param a 向量A
     * @param b 向量B
     * @return 点积值
     */
    double dotProduct(const QVector<double>& a,
                      const QVector<double>& b) const;

    /**
     * @brief 计算向量模
     * @param v 向量
     * @return 模长
     */
    double magnitude(const QVector<double>& v) const;

    mutable Stats m_stats;            ///< 统计信息(mutable支持const方法)
    double m_timeSum;                 ///< 累计处理时间
};

#endif // COSINEDISTANCE_H
