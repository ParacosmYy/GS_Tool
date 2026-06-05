/**
 * @file IndexedDeque.cpp
 * @brief 索引双端队列实现 — 环形缓冲区/自动扩容
 */

#include "utils/deque/IndexedDeque.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param initialCapacity 初始容量 @param parent 父对象 */
IndexedDeque::IndexedDeque(int initialCapacity, QObject* parent)
    : QObject(parent)
    , m_buffer(initialCapacity > 0 ? initialCapacity : 64, 0.0)
    , m_head(0)
    , m_tail(0)
    , m_count(0)
    , m_timeSum(0.0)
{
    m_stats.capacity = m_buffer.size();
}

/** @brief 前端插入 @param value 元素值 */
void IndexedDeque::pushFront(double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_count >= m_buffer.size()) {
        resizeBuffer(m_buffer.size() * 2);
    }

    /* head前移并写入 */
    m_head = (m_head - 1 + m_buffer.size()) % m_buffer.size();
    m_buffer[m_head] = value;
    m_count++;

    m_stats.totalPushFront++;
    m_stats.currentSize = m_count;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalPushFront + m_stats.totalPushBack
                     + m_stats.totalPopFront + m_stats.totalPopBack;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit elementPushed(true, value);
}

/** @brief 后端插入 @param value 元素值 */
void IndexedDeque::pushBack(double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_count >= m_buffer.size()) {
        resizeBuffer(m_buffer.size() * 2);
    }

    m_buffer[m_tail] = value;
    m_tail = (m_tail + 1) % m_buffer.size();
    m_count++;

    m_stats.totalPushBack++;
    m_stats.currentSize = m_count;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalPushFront + m_stats.totalPushBack
                     + m_stats.totalPopFront + m_stats.totalPopBack;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit elementPushed(false, value);
}

/** @brief 前端弹出 @return 弹出元素 */
double IndexedDeque::popFront()
{
    QElapsedTimer timer;
    timer.start();

    if (m_count == 0) return 0.0;

    double value = m_buffer[m_head];
    m_head = (m_head + 1) % m_buffer.size();
    m_count--;

    m_stats.totalPopFront++;
    m_stats.currentSize = m_count;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalPushFront + m_stats.totalPushBack
                     + m_stats.totalPopFront + m_stats.totalPopBack;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit elementPopped(true, value);
    return value;
}

/** @brief 后端弹出 @return 弹出元素 */
double IndexedDeque::popBack()
{
    QElapsedTimer timer;
    timer.start();

    if (m_count == 0) return 0.0;

    m_tail = (m_tail - 1 + m_buffer.size()) % m_buffer.size();
    double value = m_buffer[m_tail];
    m_count--;

    m_stats.totalPopBack++;
    m_stats.currentSize = m_count;
    m_timeSum += timer.elapsed();
    quint64 totalOps = m_stats.totalPushFront + m_stats.totalPushBack
                     + m_stats.totalPopFront + m_stats.totalPopBack;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit elementPopped(false, value);
    return value;
}

/** @brief 索引访问 @param index 索引 @return 元素值 */
double IndexedDeque::at(int index) const
{
    if (index < 0 || index >= m_count) return 0.0;
    return m_buffer[(m_head + index) % m_buffer.size()];
}

/** @brief 首元素 @return 前端元素 */
double IndexedDeque::front() const
{
    return (m_count > 0) ? m_buffer[m_head] : 0.0;
}

/** @brief 末元素 @return 后端元素 */
double IndexedDeque::back() const
{
    if (m_count == 0) return 0.0;
    int idx = (m_tail - 1 + m_buffer.size()) % m_buffer.size();
    return m_buffer[idx];
}

/** @brief 当前元素数 @return 大小 */
int IndexedDeque::size() const { return m_count; }

/** @brief 是否为空 @return 空判断 */
bool IndexedDeque::isEmpty() const { return m_count == 0; }

/** @brief 清空队列 */
void IndexedDeque::clear()
{
    m_head = 0;
    m_tail = 0;
    m_count = 0;
    m_stats.currentSize = 0;
}

/** @brief 转为QVector @return 元素向量 */
QVector<double> IndexedDeque::toVector() const
{
    QVector<double> result;
    result.reserve(m_count);
    for (int i = 0; i < m_count; ++i) {
        result.append(at(i));
    }
    return result;
}

/** @brief 扩容 @param newCapacity 新容量 */
void IndexedDeque::resizeBuffer(int newCapacity)
{
    QVector<double> newBuf(newCapacity, 0.0);
    for (int i = 0; i < m_count; ++i) {
        newBuf[i] = at(i);
    }
    m_buffer = std::move(newBuf);
    m_head = 0;
    m_tail = m_count;
    m_stats.capacity = newCapacity;
}

/** @brief 重置统计 */
void IndexedDeque::resetStatistics()
{
    m_stats = Stats{};
    m_stats.capacity = m_buffer.size();
    m_timeSum = 0.0;
}
