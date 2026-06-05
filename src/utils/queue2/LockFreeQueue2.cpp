/**
 * @file LockFreeQueue2.cpp
 * @brief LockFreeQueue2 实现 — 无锁SPSC环形缓冲区 + 批量操作
 *
 * SPSC无锁队列原理:
 * - 生产者仅写入m_tail, 消费者仅写入m_head
 * - 使用QAtomicInt保证内存可见性(Release/Acquire语义)
 * - 环形索引: index = pos & (capacity - 1), 要求capacity为2的幂
 * - 满条件: (tail - head) == capacity
 * - 空条件: tail == head
 */

#include "utils/queue2/LockFreeQueue2.h"

#include <QVector>
#include <QtMath>
#include <cstring>

// ── 构造 / 析构 ──

/**
 * @brief 构造函数, 分配2的幂大小的环形缓冲区
 * @param capacity 缓冲区容量
 * @param parent   父QObject
 */
LockFreeQueue2::LockFreeQueue2(int capacity, QObject* parent)
    : QObject(parent)
    , m_capacity(nextPowerOf2(qMax(4, capacity)))
    , m_mask(m_capacity - 1)
{
    setObjectName(QStringLiteral("LockFreeQueue2"));
    m_buffer = new double[m_capacity];
    std::memset(m_buffer, 0, sizeof(double) * static_cast<size_t>(m_capacity));
    m_head.storeRelaxed(0);
    m_tail.storeRelaxed(0);
    m_timer.start();
}

/**
 * @brief 析构函数, 释放环形缓冲区
 */
LockFreeQueue2::~LockFreeQueue2()
{
    delete[] m_buffer;
    m_buffer = nullptr;
}

// ── 单元素操作 ──

/**
 * @brief 入队一个元素
 *
 * SPSC无锁: 检查tail-head < capacity(不满), 写入buffer[tail & mask], tail++。
 * 使用Release内存序保证写入buffer在tail更新之前完成。
 * @param value 待入队的值
 * @return true = 成功, false = 队列已满
 */
bool LockFreeQueue2::push(const double& value)
{
    int tail = m_tail.loadRelaxed();
    int head = m_head.loadAcquire(); // Acquire: 确保看到consumer的最新head

    /* 满条件检查 */
    if (tail - head >= m_capacity) {
        emit queueFull(m_capacity);
        return false;
    }

    /* 写入数据 */
    m_buffer[tail & m_mask] = value;

    /* Release: 保证buffer写入在tail更新之前对consumer可见 */
    m_tail.storeRelease(tail + 1);

    /* 更新统计 */
    ++m_stats.totalPushed;
    return true;
}

/**
 * @brief 出队一个元素
 *
 * SPSC无锁: 检查tail > head(不空), 读取buffer[head & mask], head++。
 * 使用Acquire内存序保证读取head在读取buffer之前完成。
 * @return 出队的值; 空队列返回NaN
 */
double LockFreeQueue2::pop()
{
    int head = m_head.loadRelaxed();
    int tail = m_tail.loadAcquire(); // Acquire: 确保看到producer的最新tail

    /* 空条件检查 */
    if (head >= tail) {
        emit queueEmpty();
        return qQNaN();
    }

    /* 读取数据 */
    double value = m_buffer[head & m_mask];

    /* Release: 保证buffer读取在head更新之前完成 */
    m_head.storeRelease(head + 1);

    /* 更新统计 */
    ++m_stats.totalPopped;
    return value;
}

// ── 批量操作 ──

/**
 * @brief 批量入队
 *
 * 依次入队直到队列满或全部入队, 减少多次push的原子操作开销。
 * @param values 待入队的值列表
 * @return 实际入队的元素数量
 */
int LockFreeQueue2::pushBatch(const QVector<double>& values)
{
    if (values.isEmpty()) return 0;

    m_timer.restart();

    int tail = m_tail.loadRelaxed();
    int head = m_head.loadAcquire();
    int available = m_capacity - (tail - head);
    int count = qMin(values.size(), available);

    for (int i = 0; i < count; ++i) {
        m_buffer[(tail + i) & m_mask] = values[i];
    }

    if (count > 0) {
        m_tail.storeRelease(tail + count);
        m_stats.totalPushed += static_cast<quint64>(count);
    }

    /* 更新平均耗时 */
    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
    quint64 n = m_stats.totalPushed + m_stats.totalPopped;
    if (n > 0) {
        m_stats.avgProcessingTimeMs =
            (m_stats.avgProcessingTimeMs * static_cast<double>(n - 1)
             + elapsed) / static_cast<double>(n);
    }

    return count;
}

/**
 * @brief 批量出队
 *
 * 依次出队直到队列为空或达到maxCount, 减少多次pop的原子操作开销。
 * @param maxCount 最大出队数量
 * @return 出队的值列表
 */
QVector<double> LockFreeQueue2::popBatch(int maxCount)
{
    QVector<double> result;
    if (maxCount <= 0) return result;

    m_timer.restart();

    int head = m_head.loadRelaxed();
    int tail = m_tail.loadAcquire();
    int available = tail - head;
    int count = qMin(maxCount, available);

    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.append(m_buffer[(head + i) & m_mask]);
    }

    if (count > 0) {
        m_head.storeRelease(head + count);
        m_stats.totalPopped += static_cast<quint64>(count);
    }

    /* 更新平均耗时 */
    double elapsed = static_cast<double>(m_timer.nsecsElapsed()) / 1e6;
    quint64 n = m_stats.totalPushed + m_stats.totalPopped;
    if (n > 0) {
        m_stats.avgProcessingTimeMs =
            (m_stats.avgProcessingTimeMs * static_cast<double>(n - 1)
             + elapsed) / static_cast<double>(n);
    }

    return result;
}

// ── 状态查询 ──

/**
 * @brief 当前队列元素数量(近似值)
 *
 * SPSC场景下如果只在单侧调用则为精确值;
 * 跨线程同时读写时为近似值。
 * @return 元素数
 */
int LockFreeQueue2::size() const
{
    int tail = m_tail.loadRelaxed();
    int head = m_head.loadRelaxed();
    return qMax(0, tail - head);
}

/** @brief 队列是否为空 */
bool LockFreeQueue2::isEmpty() const
{
    return m_tail.loadRelaxed() == m_head.loadRelaxed();
}

/** @brief 队列容量 */
int LockFreeQueue2::capacity() const
{
    return m_capacity;
}

// ── 统计 ──

LockFreeQueue2::Stats LockFreeQueue2::stats() const
{
    return m_stats;
}

void LockFreeQueue2::resetStatistics()
{
    m_stats = Stats{};
}

// ── 私有方法 ──

/**
 * @brief 向上取整到2的幂
 *
 * 使用位运算高效计算: 不断将低半位置1后+1。
 * @param v 输入值
 * @return >= v的最小2的幂
 */
int LockFreeQueue2::nextPowerOf2(int v)
{
    if (v <= 0) return 1;
    --v;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}
