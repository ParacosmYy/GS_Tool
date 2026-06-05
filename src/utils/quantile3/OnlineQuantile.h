/**
 * @file OnlineQuantile.h
 * @brief 在线分位数估计器
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 在线分位数估计器
 *
 * 使用GK算法(Greenwald-Khanna)进行流式分位数估计,
 * 支持任意分位数查询和滑动窗口。
 */
class OnlineQuantile : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInsertions = 0;        ///< 总插入次数
        int totalQueries = 0;           ///< 总查询次数
        int tupleCount = 0;             ///< 当前元组数
        double avgProcessingTimeMs = 0.0;
    };

    explicit OnlineQuantile(double epsilon = 0.01, QObject* parent = nullptr);

    /**
     * @brief 插入一个观测值
     * @param value 观测值
     */
    void insert(double value);

    /**
     * @brief 查询指定分位数值
     * @param q 分位数(0~1)
     * @return 估计值
     */
    double query(double q) const;

    /**
     * @brief 批量查询多个分位数
     */
    QMap<double, double> queryBatch(const QVector<double>& quantiles) const;

    /**
     * @brief 中位数
     */
    double median() const;

    /**
     * @brief 四分位数
     */
    double quartile(int q) const;

    /**
     * @brief 清空
     */
    void clear();

    /**
     * @brief 观测值数量
     */
    int count() const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void valueInserted(double value, int totalCount);

private:
    /** @brief GK元组 */
    struct Tuple {
        double value;   ///< 值
        int g;          ///< 与前驱的最小rank差
        int delta;      ///< 最大rank与最小rank之差
    };

    double m_epsilon;
    QVector<Tuple> m_summary;
    int m_n;
    Stats m_stats;
    double m_timeSum = 0.0;

    void compress();
    int findRank(double q) const;
};
