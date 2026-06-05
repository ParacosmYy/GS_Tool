/**
 * @file MassEstimator.h
 * @brief 加权均值/方差估计器 — 指数/均匀加权在线估计
 *
 * 功能: 支持带权重值的在线均值与方差估计，
 *       提供标准差、样本计数、重置等接口，统计更新次数与平均耗时。
 */
#ifndef MASSESTIMATOR_H
#define MASSESTIMATOR_H

#include <QObject>
#include <QElapsedTimer>
#include <cmath>

/**
 * @brief 加权均值/方差在线估计器
 */
class MassEstimator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalUpdates = 0;        ///< 累计更新次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(毫秒)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MassEstimator(QObject* parent = nullptr);

    /**
     * @brief 更新估计器(加入新样本)
     * @param value 样本值
     * @param weight 权重(>0)
     */
    void update(double value, double weight);

    /**
     * @brief 获取当前加权均值
     * @return 加权均值
     */
    double mean() const;

    /**
     * @brief 获取当前加权方差
     * @return 加权方差
     */
    double variance() const;

    /**
     * @brief 获取当前加权标准差
     * @return 加权标准差
     */
    double stdDev() const;

    /**
     * @brief 获取已处理样本数
     * @return 样本计数
     */
    quint64 count() const;

    /**
     * @brief 重置估计器状态(清空所有数据)
     */
    void reset();

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 估计值更新 @param mean 当前均值 @param variance 当前方差 */
    void estimateUpdated(double mean, double variance);

private:
    double m_weightSum;                ///< 权重累计和
    double m_weightedValueSum;         ///< 加权值累计和
    double m_weightedSqSum;            ///< 加权平方值累计和
    double m_lastMean;                 ///< 上一次均值(缓存)
    double m_lastVariance;             ///< 上一次方差(缓存)
    quint64 m_count;                   ///< 样本计数
    mutable Stats m_stats;             ///< 统计信息(mutable支持const方法)
    double m_timeSum;                  ///< 累计处理时间
};

#endif // MASSESTIMATOR_H
