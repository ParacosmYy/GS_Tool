/**
 * @file XoshiroGenerator.h
 * @brief Xoshiro256** 伪随机数生成器 — 高质量高速随机数
 *
 * 功能: 实现Xoshiro256**算法, 提供64位整数、浮点数、
 *       高斯分布、Fisher-Yates洗牌和无放回采样。
 *       默认使用QRandomGenerator进行种子初始化。
 *
 * 协作: StatDistribution(分布拟合) / AnomalyDetector(蒙特卡洛检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QRandomGenerator>

/**
 * @brief Xoshiro256** 伪随机数生成器
 */
class XoshiroGenerator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalGenerated = 0;       ///< 累计生成随机数个数
        double  avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    /**
     * @brief 默认构造函数, 使用QRandomGenerator自动播种
     * @param parent 父对象
     *
     * 使用系统熵源初始化256位状态, 确保每次运行产生不同序列。
     */
    explicit XoshiroGenerator(QObject* parent = nullptr);

    /**
     * @brief 手动播种
     * @param s 种子值(会被splitmix64扩展为4个状态字)
     *
     * 相同种子产生相同序列, 适用于可重复实验。
     */
    void seed(quint64 s);

    /**
     * @brief 生成下一个64位随机数
     * @return [0, 2^64-1] 范围的随机整数
     */
    quint64 next();

    /**
     * @brief 生成[0, 1)范围的随机双精度浮点数
     * @return [0.0, 1.0) 随机浮点数
     */
    double nextDouble();

    /**
     * @brief 生成[min, max]范围的随机整数
     * @param min 最小值(包含)
     * @param max 最大值(包含)
     * @return [min, max] 范围随机整数
     */
    int nextInt(int min, int max);

    /**
     * @brief 生成标准正态分布随机数(Box-Muller变换)
     * @return N(0,1) 随机数
     *
     * 使用Box-Muller变换将均匀分布转换为正态分布。
     */
    double nextGaussian();

    /**
     * @brief Fisher-Yates原位洗牌
     * @param data 待洗牌数据(会被原地修改)
     *
     * 使用Fisher-Yates算法保证每种排列等概率出现。
     */
    void shuffle(QVector<double>& data);

    /**
     * @brief 无放回采样
     * @param population 总体数据
     * @param k 采样数量(不超过population大小)
     * @return 采样结果(顺序随机)
     *
     * 使用蓄水池采样算法, 时间复杂度O(n), 空间复杂度O(k)。
     */
    QVector<double> sample(const QVector<double>& population, int k);

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 洗牌完成 @param count 数据量 */
    void shuffleCompleted(int count);

    /** @brief 采样完成 @param sampleSize 采样大小 */
    void sampleCompleted(int sampleSize);

private:
    /**
     * @brief SplitMix64生成器(用于种子扩展)
     * @return 64位伪随机数
     */
    quint64 splitmix64();

    quint64 m_state[4];            ///< Xoshiro256**内部状态(256位)
    quint64 m_splitState;          ///< SplitMix64状态(种子扩展用)
    bool    m_hasSpare;            ///< Box-Muller备用值标志
    double  m_spare;               ///< Box-Muller备用正态值

    Stats  m_stats;                ///< 统计信息
    double m_timeSum;              ///< 处理时间累加器
};
