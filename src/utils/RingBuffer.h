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
    explicit RingBuffer(int capacity = 1024)
        : m_capacity(capacity), m_head(0), m_tail(0), m_count(0)
    {
        m_buffer.resize(capacity);
    }

    // 写入一个元素，满时覆盖最旧的
    void push(const T& item)
    {
        QMutexLocker locker(&m_mutex);
        m_buffer[m_tail] = item;
        m_tail = (m_tail + 1) % m_capacity;
        if (m_count == m_capacity) {
            m_head = (m_head + 1) % m_capacity;  // 覆盖最旧
        } else {
            m_count++;
        }
    }

    // 读取一个元素，返回是否成功
    bool pop(T& item)
    {
        QMutexLocker locker(&m_mutex);
        if (m_count == 0) return false;
        item = m_buffer[m_head];
        m_head = (m_head + 1) % m_capacity;
        m_count--;
        return true;
    }

    // 获取当前元素数量
    int count() const
    {
        QMutexLocker locker(&m_mutex);
        return m_count;
    }

    // 是否为空
    bool isEmpty() const
    {
        QMutexLocker locker(&m_mutex);
        return m_count == 0;
    }

    // 是否已满
    bool isFull() const
    {
        QMutexLocker locker(&m_mutex);
        return m_count == m_capacity;
    }

    // 清空缓冲区
    void clear()
    {
        QMutexLocker locker(&m_mutex);
        m_head = 0;
        m_tail = 0;
        m_count = 0;
    }

    // 修改容量(会清空现有数据)
    void setCapacity(int capacity)
    {
        QMutexLocker locker(&m_mutex);
        m_capacity = capacity;
        m_buffer.resize(capacity);
        // 内联 clear() 逻辑，避免对同一非递归 mutex 二次加锁导致死锁
        m_head = 0;
        m_tail = 0;
        m_count = 0;
    }

    // 按索引读取元素(0=最旧, count-1=最新)，不删除
    bool at(int index, T& item) const
    {
        QMutexLocker locker(&m_mutex);
        if (index < 0 || index >= m_count) return false;
        item = m_buffer[(m_head + index) % m_capacity];
        return true;
    }

private:
    mutable QMutex m_mutex;
    QVector<T> m_buffer;
    int m_capacity;
    int m_head;      // 读位置
    int m_tail;      // 写位置
    int m_count;     // 当前元素数
};

#endif // RINGBUFFER_H
