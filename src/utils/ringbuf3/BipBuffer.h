/**
 * @file BipBuffer.h
 * @brief 双端缓冲区(Bip Buffer)
 */

#pragma once

#include <QObject>
#include <QByteArray>

/**
 * @class BipBuffer
 * @brief 双端缓冲区 — 无锁单生产者单消费者环形缓冲
 *
 * 支持变长消息、零拷贝预留/提交、内存连续保证。
 * 适用于高性能数据流、串口缓冲、音频缓冲等场景。
 */
class BipBuffer : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalWrites = 0;       /**< 总写入次数 */
        int totalReads = 0;        /**< 总读取次数 */
        long long totalBytesWritten = 0; /**< 总写入字节数 */
        long long totalBytesRead = 0;    /**< 总读取字节数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit BipBuffer(int capacity = 65536, QObject* parent = nullptr);

    /**
     * @brief 预留写空间
     * @param size 请求大小
     * @return 可写区域指针和大小(可能小于请求)
     */
    QPair<char*, int> reserveWrite(int size);

    /**
     * @brief 提交写入
     * @param size 实际写入大小
     */
    void commitWrite(int size);

    /**
     * @brief 获取可读区域
     * @return 可读区域指针和大小
     */
    QPair<const char*, int> availableRead() const;

    /**
     * @brief 提交读取
     * @param size 实际读取大小
     */
    void commitRead(int size);

    /**
     * @brief 直接写入数据
     * @param data 输入数据
     * @return 实际写入字节数
     */
    int write(const QByteArray& data);

    /**
     * @brief 直接读取数据
     * @param maxSize 最大读取大小
     * @return 读取的数据
     */
    QByteArray read(int maxSize);

    /** @brief 缓冲区容量 */
    int capacity() const;

    /** @brief 可读字节数 */
    int available() const;

    /** @brief 可写字节数 */
    int freeSpace() const;

    /** @brief 是否为空 */
    bool isEmpty() const;

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
    QByteArray m_buffer;
    int m_aStart, m_aEnd;
    int m_bStart, m_bEnd;
    int m_writeRegion; /**< 0=未分配, 1=A区, 2=B区 */

    Stats m_stats;
    double m_timeSum;
};
