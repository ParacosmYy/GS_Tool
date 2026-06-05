/**
 * @file RingBufferAtomic.h
 * @brief 原子环形缓冲区(Atomic Ring Buffer)
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <atomic>

/**
 * @class RingBufferAtomic
 * @brief 原子环形缓冲区 — 无锁单生产者单消费者SPSC缓冲
 *
 * 使用原子操作实现无锁读写, 支持变长消息。
 * 适用于实时数据流、音频/视频缓冲、高速串口数据。
 */
class RingBufferAtomic : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalWrites = 0;     /**< 总写入次数 */
        int totalReads = 0;      /**< 总读取次数 */
        long long totalBytesWritten = 0; /**< 总写入字节 */
        long long totalBytesRead = 0;    /**< 总读取字节 */
    };

    /** @brief 构造函数 */
    explicit RingBufferAtomic(int capacity = 65536, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~RingBufferAtomic();

    /**
     * @brief 写入数据
     * @param data 输入数据
     * @return 实际写入字节数
     */
    int write(const QByteArray& data);

    /**
     * @brief 读取数据
     * @param maxSize 最大读取大小
     * @return 读取的数据
     */
    QByteArray read(int maxSize);

    /**
     * @brief 写入单个字节
     * @param byte 字节值
     * @return 是否成功
     */
    bool writeByte(quint8 byte);

    /**
     * @brief 读取单个字节
     * @param byte 输出字节
     * @return 是否成功
     */
    bool readByte(quint8& byte);

    /** @brief 可读字节数 */
    int available() const;

    /** @brief 可写字节数 */
    int freeSpace() const;

    /** @brief 缓冲区容量 */
    int capacity() const;

    /** @brief 是否为空 */
    bool isEmpty() const;

    /** @brief 是否已满 */
    bool isFull() const;

    /** @brief 清空缓冲区 */
    void clear();

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 数据写入信号 */
    void dataWritten(int bytes);

private:
    char* m_buffer;
    int m_capacity;
    int m_mask;
    std::atomic<int> m_head; /**< 写位置 */
    std::atomic<int> m_tail; /**< 读位置 */
    Stats m_stats;
};
