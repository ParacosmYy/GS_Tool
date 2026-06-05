/**
 * @file LockFreeQueue2.h
 * @brief 无锁SPSC队列 — 单生产者单消费者环形缓冲区 + 批量操作
 *
 * 实现基于原子操作的单一生产者-单一消费者(SPSC)无锁队列，
 * 使用环形缓冲区和内存序(memory order)保证无等待(wait-free)的
 * push/pop操作。支持批量push/pop以减少原子操作开销。
 * 适用于高频数据采集、实时波形数据传输等嵌入式调试场景。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QAtomicInteger>
#include <QElapsedTimer>

/**
 * @class LockFreeQueue2
 * @brief 无锁SPSC队列 — 批量操作环形缓冲区
 *
 * 典型用法:
 * @code
 *   LockFreeQueue2 queue(65536);
 *   queue.push(3.14);
 *   queue.pushBatch(dataVector);
 *   double val = queue.pop();
 *   QVector<double> batch = queue.popBatch(100);
 * @endcode
 */
class LockFreeQueue2 : public QObject {
    Q_OBJECT

public:
    /** @brief 队列操作统计结构 */
    struct Stats {
        quint64 totalPushed            = 0;    ///< push操作总元素数
        quint64 totalPopped            = 0;    ///< pop操作总元素数
        double  avgProcessingTimeMs    = 0.0;  ///< 平均操作耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param capacity 环形缓冲区容量(默认65536, 自动向上取整到2的幂)
     * @param parent   父对象
     */
    explicit LockFreeQueue2(int capacity = 65536, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~LockFreeQueue2() override;

    // ── 单元素操作 ──

    /**
     * @brief 入队一个元素(SPSC无锁)
     * @param value 待入队的值
     * @return true = 成功入队, false = 队列已满
     */
    bool push(const double& value);

    /**
     * @brief 出队一个元素(SPSC无锁)
     * @return 出队的值; 队列为空时返回NaN
     */
    double pop();

    // ── 批量操作 ──

    /**
     * @brief 批量入队
     *
     * 从values中依次入队直到队列满或全部入队。
     * @param values 待入队的值列表
     * @return 实际入队的元素数量
     */
    int pushBatch(const QVector<double>& values);

    /**
     * @brief 批量出队
     *
     * 从队列中依次出队最多maxCount个元素。
     * @param maxCount 最大出队数量
     * @return 出队的值列表(可能少于maxCount)
     */
    QVector<double> popBatch(int maxCount);

    // ── 状态查询 ──

    /** @brief 当前队列中的元素数量(近似值, 多线程下非精确) @return 元素数 */
    int size() const;

    /** @brief 队列是否为空 @return true = 空 */
    bool isEmpty() const;

    /** @brief 队列容量 @return 环形缓冲区总容量 */
    int capacity() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 @return Stats结构体副本 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 队列已满, push失败时发射 @param currentSize 当前队列大小 */
    void queueFull(int currentSize);
    /** @brief 队列为空, pop失败时发射 */
    void queueEmpty();

private:
    /**
     * @brief 向上取整到2的幂
     * @param v 输入值
     * @return >= v的最小2的幂
     */
    static int nextPowerOf2(int v);

    double* m_buffer;               ///< 环形缓冲区
    int     m_capacity;             ///< 容量(2的幂)
    int     m_mask;                 ///< 掩码 = capacity - 1

    alignas(64) QAtomicInt m_head;  ///< 读指针(consumer侧)
    alignas(64) QAtomicInt m_tail;  ///< 写指针(producer侧)

    Stats  m_stats;                 ///< 统计数据
    QElapsedTimer m_timer;          ///< 耗时计时器
};
