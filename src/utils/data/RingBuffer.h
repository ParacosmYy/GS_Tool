/**
 * @file RingBuffer.h
 * @brief 线程安全环形缓冲区模板 — 固定容量，覆盖最旧数据
 *
 * 模板参数T需支持默认构造和赋值操作。
 * 内部使用QMutex保证线程安全，适用于多生产者-单消费者场景。
 * 当缓冲区满时新数据覆盖最旧数据(覆盖策略)。
 */
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <QVector>
#include <QMutex>
#include <QMutexLocker>

// 环形缓冲区 - 固定大小，先进先出，自动覆盖最旧数据
// 线程安全，适用于生产者-消费者场景
template<typename T>
class RingBuffer {
public:
    /**
     * @brief 构造函数
     * @param capacity 缓冲区容量，最小为1
     */
    explicit RingBuffer(int capacity = 1024)
        : m_capacity(qMax(capacity, 1)), m_head(0), m_tail(0), m_count(0)
    {
        m_buffer.resize(m_capacity);
    }

    /** @brief 写入一个元素，满时覆盖最旧的 @param item 要写入的元素 */
    void push(const T& item)
    {
        QMutexLocker locker(&m_mutex);
        ++m_totalPushes;
        m_buffer[m_tail] = item;
        m_tail = (m_tail + 1) % m_capacity;
        if (m_count == m_capacity) {
            ++m_totalOverflows;
            m_head = (m_head + 1) % m_capacity;  // 覆盖最旧
        } else {
            m_count++;
        }
        if (m_count > m_peakUsage) {
            m_peakUsage = m_count;
        }
    }

    /** @brief 读取一个元素 @param item 输出参数 @return true成功, false缓冲区为空 */
    bool pop(T& item)
    {
        QMutexLocker locker(&m_mutex);
        if (m_count == 0) return false;
        ++m_totalPops;
        item = m_buffer[m_head];
        m_head = (m_head + 1) % m_capacity;
        m_count--;
        return true;
    }

    /** @brief 获取当前元素数量 @return 元素数量 */
    int count() const
    {
        QMutexLocker locker(&m_mutex);
        return m_count;
    }

    /** @brief 检查缓冲区是否为空 @return true为空 */
    bool isEmpty() const
    {
        QMutexLocker locker(&m_mutex);
        return m_count == 0;
    }

    /** @brief 检查缓冲区是否已满 @return true已满 */
    bool isFull() const
    {
        QMutexLocker locker(&m_mutex);
        return m_count == m_capacity;
    }

    /** @brief 清空缓冲区 */
    void clear()
    {
        QMutexLocker locker(&m_mutex);
        m_head = 0;
        m_tail = 0;
        m_count = 0;
    }

    /** @brief 修改容量(会清空现有数据) @param capacity 新容量 */
    void setCapacity(int capacity)
    {
        QMutexLocker locker(&m_mutex);
        m_capacity = qMax(capacity, 1);
        m_buffer.resize(m_capacity);
        // 内联 clear() 逻辑，避免对同一非递归 mutex 二次加锁导致死锁
        m_head = 0;
        m_tail = 0;
        m_count = 0;
    }

    /** @brief 按索引读取元素(0=最旧, count-1=最新)，不删除 @param index 索引 @param item 输出参数 @return true成功, false索引越界 */
    bool at(int index, T& item) const
    {
        QMutexLocker locker(&m_mutex);
        if (index < 0 || index >= m_count) return false;
        item = m_buffer[(m_head + index) % m_capacity];
        return true;
    }

    // ---- 内联统计计数接口 ----

    /** @brief 获取累计push写入次数 @return 写入总次数 */
    quint64 totalPushes() const { return m_totalPushes; }

    /** @brief 获取累计pop读取次数 @return 读取总次数 */
    quint64 totalPops() const { return m_totalPops; }

    /** @brief 获取累计覆盖溢出次数(缓冲区满时新数据覆盖旧数据) @return 溢出次数 */
    quint64 totalOverflows() const { return m_totalOverflows; }

    /** @brief 获取历史峰值使用量(元素数) @return 峰值元素数量 */
    int peakUsage() const { return m_peakUsage; }

    /** @brief 获取缓冲区总容量 @return 容量 */
    int capacity() const { return m_capacity; }

    /** @brief 重置所有统计计数器(pops/pushes/overflows/peak归零) */
    void resetStats()
    {
        QMutexLocker locker(&m_mutex);
        m_totalPushes = 0;
        m_totalPops = 0;
        m_totalOverflows = 0;
        m_peakUsage = 0;
    }

private:
    mutable QMutex m_mutex;       ///< 线程安全互斥锁
    QVector<T> m_buffer;          ///< 底层存储容器
    int m_capacity;               ///< 环形缓冲区总容量
    int m_head;                   ///< 读位置索引（最旧元素）
    int m_tail;                   ///< 写位置索引（下一个写入位置）
    int m_count;                  ///< 当前有效元素数量

    // ---- 内联统计计数器 ----
    quint64 m_totalPushes = 0;    ///< 累计push写入次数
    quint64 m_totalPops = 0;      ///< 累计pop读取次数
    quint64 m_totalOverflows = 0; ///< 累计覆盖溢出次数(缓冲区满时push)
    int m_peakUsage = 0;          ///< 历史峰值使用量(元素数)
};

#endif // RINGBUFFER_H
