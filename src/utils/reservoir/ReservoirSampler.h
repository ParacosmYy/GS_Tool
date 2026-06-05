/**
 * @file ReservoirSampler.h
 * @brief 水塘采样器 — 流式随机采样(Algorithm R)
 *
 * 功能: 实现经典水塘采样算法(Algorithm R)，
 *       支持逐个/批量添加数据，可配置水塘大小，
 *       统计总处理数、采样数与平均耗时。
 */
#ifndef RESERVOIRSAMPLER_H
#define RESERVOIRSAMPLER_H

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <QRandomGenerator>

/**
 * @brief 水塘采样器(Algorithm R)
 */
class ReservoirSampler : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSamples = 0;        ///< 水塘中当前样本数
        quint64 totalProcessed = 0;      ///< 累计处理的数据总数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(毫秒)
    };

    /**
     * @brief 构造函数
     * @param reservoirSize 水塘大小(默认100)
     * @param parent 父对象
     */
    explicit ReservoirSampler(int reservoirSize = 100,
                              QObject* parent = nullptr);

    /**
     * @brief 添加一个数据点到采样流
     * @param value 数据值
     */
    void add(double value);

    /**
     * @brief 批量添加数据点
     * @param values 数据值列表
     */
    void addBatch(const QVector<double>& values);

    /**
     * @brief 获取当前水塘中的样本
     * @return 采样结果
     */
    QVector<double> samples() const;

    /**
     * @brief 重置采样器(清空水塘)
     */
    void reset();

    /**
     * @brief 设置水塘大小
     * @param size 新的水塘大小(会清空已有样本)
     */
    void setReservoirSize(int size);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 采样数据添加 @param value 添加的值 */
    void sampleAdded(double value);

private:
    int m_reservoirSize;               ///< 水塘大小
    QVector<double> m_reservoir;       ///< 水塘样本容器
    QRandomGenerator m_rng;            ///< 随机数生成器
    mutable Stats m_stats;             ///< 统计信息(mutable支持const方法)
    double m_timeSum;                  ///< 累计处理时间
};

#endif // RESERVOIRSAMPLER_H
