/**
 * @file MetropolisHastings.h
 * @brief Metropolis-Hastings采样器(Metropolis-Hastings Sampler)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @class MetropolisHastings
 * @brief Metropolis-Hastings采样器 — MCMC采样算法
 *
 * 支持自定义目标分布和提议分布、自适应步长、
 * 链诊断(自相关、有效样本量)。
 * 适用于贝叶斯推断、复杂分布采样等场景。
 */
class MetropolisHastings : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSamples = 0;    /**< 总采样数 */
        int totalAccepted = 0;   /**< 总接受数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit MetropolisHastings(QObject* parent = nullptr);

    /**
     * @brief 设置目标分布对数概率
     * @param logPdf 对数概率密度函数
     */
    void setTargetDistribution(std::function<double(double)> logPdf);

    /**
     * @brief 设置提议分布标准差
     * @param sigma 高斯提议标准差
     */
    void setProposalWidth(double sigma);

    /**
     * @brief 运行采样
     * @param initial 起始值
     * @param nSamples 采样数
     * @param burnIn 燃烧期(丢弃前N个)
     * @return 采样链
     */
    QVector<double> sample(double initial, int nSamples, int burnIn = 1000);

    /**
     * @brief 计算自相关函数
     * @param chain 采样链
     * @param maxLag 最大滞后阶数
     * @return 自相关值列表
     */
    static QVector<double> autocorrelation(const QVector<double>& chain, int maxLag = 50);

    /**
     * @brief 计算有效样本量(ESS)
     * @param chain 采样链
     * @return 有效样本量
     */
    static int effectiveSampleSize(const QVector<double>& chain);

    /** @brief 接受率 */
    double acceptanceRate() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 采样完成信号 */
    void samplingCompleted(int samples, double acceptanceRate);

private:
    std::function<double(double)> m_logPdf;
    double m_proposalWidth;
    int m_accepted;
    int m_total;

    Stats m_stats;
    double m_timeSum;
};
