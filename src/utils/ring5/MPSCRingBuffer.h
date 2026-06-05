/**
 * @file MPSCRingBuffer.h
 * @brief 多生产者单消费者无锁环形缓冲区
 *
 * 功能: 高性能MPSC环形缓冲区, 多线程生产者无锁写入,
 *       单消费者线程读取, 适用于日志/事件收集等场景。
 *
 * 协作: DataPipeline(数据管道) / SerialDataLogger(日志记录)
 */

#pragma once

#include <QObject>
#include <QAtomicInt>
#include <QVector>

/**
 * @brief 多生产者单消费者无锁环形缓冲区
 *
 * 设计要点:
 * - 生产者端: CAS无锁写入, 支持多线程并发
 * - 消费者端: 单线程顺序读取, 无需同步
 * - 内存序: 使用acquire/release语义保证可见性
 * - 批量操作: 支持批量读写减少竞争
 */
class MPSCRingBuffer : public QObject
{
    Q_OBJECT

public:
    /** @brief 槽位状态 */
    enum class SlotState : int {
        Free = 0,           ///< 可写入
        Writing = 1,        ///< 正在写入
        Ready = 2,          ///< 可读取
        Reading = 3         ///< 正在读取
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalItemsWritten = 0;          ///< 累计写入项目数
        int totalItemsRead = 0;             ///< 累计读取项目数
        int totalWriteRetries = 0;          ///< 累计写入重试(CAS冲突)
        int totalReadUnderruns = 0;         ///< 累计读取下溢
        int totalOverflows = 0;             ///< 累计溢出次数
        double avgBatchSize = 0.0;          ///< 平均批量大小
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit MPSCRingBuffer(int capacity = 1024, QObject* parent = nullptr);

    ~MPSCRingBuffer() override;

    /**
     * @brief 无锁写入单个数据项
     * @param data 数据值
     * @return true=写入成功, false=缓冲区满
     */
    bool write(double data);

    /**
     * @brief 无锁批量写入
     * @param data 数据数组
     * @return 实际写入的项目数
     */
    int writeBatch(const QVector<double>& data);

    /**
     * @brief 消费者端读取单个数据项
     * @param value 输出值
     * @return true=读取成功, false=缓冲区空
     */
    bool read(double& value);

    /**
     * @brief 消费者端批量读取
     * @param maxItems 最大读取数
     * @return 读取到的数据数组
     */
    QVector<double> readBatch(int maxItems);

    /**
     * @brief 获取可读取的项目数(近似值)
     * @return 可读项目数
     */
    int available() const;

    /**
     * @brief 获取剩余可写空间(近似值)
     * @return 可写空间
     */
    int freeSpace() const;

    /**
     * @brief 缓冲区是否为空(近似)
     */
    bool isEmpty() const;

    /**
     * @brief 重置缓冲区
     */
    void reset();

    /**
     * @brief 获取缓冲区容量
     */
    int capacity() const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 缓冲区溢出信号 @param dropped 丢弃的项数 */
    void overflow(int dropped);

    /** @brief 批量写入完成 @param count 写入数量 */
    void batchWritten(int count);

private:
    /**
     * @brief 获取下一个生产者写入位置
     * @return 写入位置索引, -1表示缓冲区满
     */
    int acquireWriteSlot();

    /**
     * @brief 提交写入完成
     * @param slot 槽位索引
     */
    void commitWriteSlot(int slot);

    struct Slot {
        double data = 0.0;
        QAtomicInt state;          ///< SlotState
    };

    QVector<Slot> m_buffer;
    int m_capacity;
    int m_mask;                    ///< capacity - 1 (要求capacity为2的幂)
    QAtomicInt m_writeHead;        ///< 生产者写入头(只增)
    QAtomicInt m_writeTail;        ///< 已提交写入尾
    QAtomicInt m_readHead;         ///< 消费者读取头
    QAtomicInt m_committed;        ///< 已提交数量

    Stats m_stats;
    double m_timeSum = 0.0;
    int m_batchSum = 0;
};
