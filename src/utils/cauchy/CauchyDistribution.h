/**
 * @file CauchyDistribution.h
 * @brief 柯西分布(Cauchy Distribution)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class CauchyDistribution
 * @brief 柯西分布 — 重尾概率分布
 *
 * 支持PDF/CDF/分位数/随机生成/参数估计。
 * 柯西分布无有限均值和方差, 适用于稳健统计和信号建模。
 */
class CauchyDistribution : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSamples = 0;     /**< 总采样次数 */
        int totalEvaluations = 0; /**< 总评估次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit CauchyDistribution(double location = 0.0, double scale = 1.0,
                                  QObject* parent = nullptr);

    /**
     * @brief 概率密度函数(PDF)
     * @param x 输入值
     * @return 概率密度
     */
    double pdf(double x) const;

    /**
     * @brief 累积分布函数(CDF)
     * @param x 输入值
     * @return 累积概率
     */
    double cdf(double x) const;

    /**
     * @brief 分位数函数(逆CDF)
     * @param p 概率[0,1]
     * @return 分位数值
     */
    double quantile(double p) const;

    /**
     * @brief 生成随机样本
     * @param n 样本数
     * @return 样本列表
     */
    QVector<double> sample(int n) const;

    /**
     * @brief 从样本估计参数(分位数法)
     * @param samples 样本数据
     * @return (location, scale)估计
     */
    static QPair<double, double> estimate(const QVector<double>& samples);

    /** @brief 设置位置参数 */
    void setLocation(double loc);

    /** @brief 设置尺度参数 */
    void setScale(double scale);

    double location() const;
    double scale() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 采样完成信号 */
    void sampled(int count);

private:
    double m_location;
    double m_scale;
    mutable Stats m_stats;
    mutable double m_timeSum;
};
