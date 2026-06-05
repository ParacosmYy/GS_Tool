/**
 * @file SeededRandom.h
 * @brief 可播种的确定性随机数生成器(PCG)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class SeededRandom
 * @brief PCG-based确定性随机数生成器
 *
 * 使用PCG-XSH-RR算法，提供高质量伪随机数。
 * 支持确定性播种，适合测试、模拟和可重现的随机序列。
 */
class SeededRandom : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalGenerated = 0;    /**< 总生成数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit SeededRandom(QObject* parent = nullptr);

    /**
     * @brief 设置种子
     * @param seed 种子值
     */
    void seed(quint64 seed);

    /** @brief 生成32位随机数 */
    quint32 nextUInt32();

    /** @brief 生成64位随机数 */
    quint64 nextUInt64();

    /** @brief 生成[0, 1)均匀分布double */
    double nextDouble();

    /** @brief 生成[min, max]均匀分布int */
    int nextInt(int min, int max);

    /** @brief 生成正态分布随机数(Box-Muller) */
    double nextGaussian(double mean = 0.0, double stddev = 1.0);

    /** @brief 从列表中随机选择一个元素 */
    double choice(const QVector<double>& population);

    /** @brief 无放回抽样 */
    QVector<double> sample(const QVector<double>& population, int k);

    /** @brief Fisher-Yates洗牌 */
    void shuffle(QVector<double>& data);

    /** @brief 生成指定长度的随机字节 */
    QByteArray nextBytes(int length);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 生成完成信号 */
    void generated(int count);

private:
    quint64 m_state;
    quint64 m_inc;
    bool m_hasSpare;
    double m_spare;

    Stats m_stats;
    double m_timeSum;
};
