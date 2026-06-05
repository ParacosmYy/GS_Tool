/**
 * @file ZipfGenerator.h
 * @brief Zipf分布生成器 — 基于Ziggurat/逆变换采样的Zipf随机数生成
 *
 * 功能: 生成服从Zipf分布的随机数，支持参数化的幂律指数，
 *       适用于网络流量建模、自然语言处理等场景。
 *
 * 协作: StatDistribution(统计分布) / FrequencyCounterWidget(频率统计)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <random>
#include <vector>

/**
 * @brief Zipf分布生成器 — 幂律随机数
 */
class ZipfGenerator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSamples          = 0;   ///< 累计采样次数
        quint64 totalGenerated        = 0;   ///< 累计生成数
        double  avgProcessingTimeMs   = 0.0; ///< 平均处理时间(ms)
        double  lastAlpha             = 0.0; ///< 最近使用的alpha
        int     lastN                 = 0;   ///< 最近使用的N
    };

    /**
     * @brief 构造函数
     * @param n 元素数量(1..n)
     * @param alpha 幂律指数(>0, 通常0.5~2.0)
     * @param parent 父对象
     */
    explicit ZipfGenerator(int n = 100, double alpha = 1.0,
                           QObject* parent = nullptr);

    /**
     * @brief 设置参数
     * @param n 元素数量
     * @param alpha 幂律指数
     */
    void setParams(int n, double alpha);

    /**
     * @brief 生成单个Zipf随机数
     * @return 随机秩(1..n)
     */
    int sample();

    /**
     * @brief 批量生成Zipf随机数
     * @param count 数量
     * @return 随机数列表
     */
    QVector<int> sampleBatch(int count);

    /**
     * @brief 计算Zipf概率质量函数
     * @param k 秩(1..n)
     * @return P(X=k)
     */
    double probability(int k) const;

    /**
     * @brief 计算Zipf累积分布函数
     * @param k 秩(1..n)
     * @return P(X≤k)
     */
    double cdf(int k) const;

    /**
     * @brief 生成理论概率分布
     * @return (秩列表, 概率列表)
     */
    QPair<QVector<int>, QVector<double>> theoreticalDistribution() const;

    /**
     * @brief 计算理论均值
     * @return 均值
     */
    double mean() const;

    /**
     * @brief 计算理论方差
     * @return 方差
     */
    double variance() const;

    /** @brief 获取元素数 @return N */
    int n() const { return m_n; }

    /** @brief 获取alpha @return alpha */
    double alpha() const { return m_alpha; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 批量生成完成 @param count 数量 */
    void batchGenerated(int count);

private:
    /**
     * @brief 预计算累积分布(用于逆变换采样)
     */
    void buildCDF();

    int     m_n;          ///< 元素数量
    double  m_alpha;      ///< 幂律指数

    std::mt19937 m_rng;              ///< 随机数生成器
    std::vector<double> m_cdfTable;  ///< 累积分布表(用于逆变换)
    double m_harmonicSum;            ///< 广义调和数 H_{n,alpha}

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;
};
