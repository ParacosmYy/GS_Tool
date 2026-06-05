/**
 * @file CircularPowerBuffer.h
 * @brief 2的幂次环形缓冲区 — O(1)读写高速数据缓冲
 *
 * 功能: 容量为2的幂次的固定大小环形缓冲区，利用位掩码实现O(1)
 *       读写操作，支持批量读写和溢出检测，适用于实时数据流缓冲。
 *
 * 协作: DataRingBuffer(通用环形缓冲) / DataCaptureBuffer(数据捕获)
 */
#ifndef CIRCULARPOWERBUFFER_H
#define CIRCULARPOWERBUFFER_H

#include <QObject>
#include <QVector>

/**
 * @brief 2的幂次环形缓冲区 — O(1)位掩码读写
 */
class CircularPowerBuffer : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalWrites = 0;        ///< 累计写入次数
        quint64 totalReads = 0;         ///< 累计读取次数
        quint64 totalOverflows = 0;     ///< 累计溢出次数
    };

    /**
     * @brief 构造函数
     * @param powerSize 容量为2^powerSize(默认10=1024)
     * @param parent 父对象 */
    explicit CircularPowerBuffer(int powerSize = 10, QObject* parent = nullptr);

    /** @brief 写入单个值 @param value 数值 */
    void write(double value);

    /** @brief 批量写入 @param values 数据数组 */
    void writeBatch(const QVector<double>& values);

    /** @brief 读取单个值 @return 数值(缓冲区空返回NaN) */
    double read();

    /** @brief 批量读取 @param count 读取数量 @return 数据数组 */
    QVector<double> readBatch(int count);

    /** @brief 可读数据量 @return 可读样本数 */
    int available() const;

    /** @brief 缓冲区容量 @return 容量 */
    int capacity() const;

    /** @brief 清空缓冲区 */
    void clear();

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 溢出警告 @param lostSamples 丢失样本数 */
    void overflowWarning(int lostSamples);

private:
    QVector<double> m_buffer;      ///< 数据缓冲
    int m_mask;                    ///< 位掩码(capacity-1)
    int m_capacity;                ///< 容量
    int m_head;                    ///< 写指针
    int m_tail;                    ///< 读指针
    int m_count;                   ///< 当前数据量

    Stats m_stats;                 ///< 统计
};

#endif // CIRCULARPOWERBUFFER_H
