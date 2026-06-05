/**
 * @file RingBufferEx.h
 * @brief 扩展环形缓冲区 — 固定容量循环缓冲与统计快照
 *
 * 功能: 支持覆盖写入的固定容量环形缓冲区，提供均值/标准差/
 *       最小/最大统计快照，支持批量读写操作。
 *
 * 协作: DataPipeline(数据缓存) / QueueSimulator(队列模拟)
 */
#ifndef RINGBUFFEREX_H
#define RINGBUFFEREX_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class RingBufferEx
 * @brief 带统计功能的扩展环形缓冲区
 */
class RingBufferEx : public QObject {
    Q_OBJECT

public:
    /** 缓冲区统计 */
    struct Stats {
        quint64 totalWrites = 0;              ///< 总写入次数
        quint64 totalReads = 0;               ///< 总读取次数
        quint64 totalOverwrites = 0;          ///< 总覆盖次数
        quint64 peakUsage = 0;                ///< 峰值使用量
    };

    /** 统计快照 */
    struct Snapshot {
        double mean = 0.0;                    ///< 均值
        double stddev = 0.0;                  ///< 标准差
        double min = 0.0;                     ///< 最小值
        double max = 0.0;                     ///< 最大值
        quint64 count = 0;                    ///< 当前元素数
    };

    /**
     * @brief 构造函数
     * @param capacity 缓冲区容量
     * @param parent 父对象
     */
    explicit RingBufferEx(int capacity = 1024, QObject* parent = nullptr);

    /** @brief 写入单个值(满时覆盖最旧) @param value 值 */
    void write(double value);

    /** @brief 批量写入 @param values 值列表 */
    void writeBatch(const QVector<double>& values);

    /** @brief 读取单个值(从最旧开始) @return 读取结果(有效标志, 值) */
    QPair<bool, double> read();

    /** @brief 批量读取 @param maxCount 最大读取数 @return 值列表 */
    QVector<double> readBatch(int maxCount);

    /** @brief 查看最新值(不移除) @return 最新值(有效标志, 值) */
    QPair<bool, double> peekLatest() const;

    /** @brief 获取统计快照 @return 快照 */
    Snapshot snapshot() const;

    /** @brief 当前元素数 @return 元素数 */
    int size() const;

    /** @brief 缓冲区是否为空 @return 空标志 */
    bool isEmpty() const;

    /** @brief 缓冲区是否已满 @return 满标志 */
    bool isFull() const;

    /** @brief 缓冲区容量 @return 容量 */
    int capacity() const;

    /** @brief 清空缓冲区 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 缓冲区溢出(覆盖) @param overwrittenCount 覆盖数量 */
    void overflowOccurred(quint64 overwrittenCount);

    /** @brief 数据写入完成 @param currentSize 当前大小 */
    void dataWritten(int currentSize);

private:
    QVector<double> m_buffer;             ///< 底层缓冲区
    int m_capacity;                       ///< 容量
    int m_head;                           ///< 写入位置
    int m_tail;                           ///< 读取位置
    int m_count;                          ///< 当前元素数

    Stats m_stats;
};

#endif // RINGBUFFEREX_H
