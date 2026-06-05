/**
 * @file RingBufferAtomic.cpp
 * @brief 原子环形缓冲区实现
 */

#include "RingBufferAtomic.h"
#include <cstring>

RingBufferAtomic::RingBufferAtomic(int capacity, QObject* parent)
    : QObject(parent)
    , m_capacity(1)
    , m_head(0)
    , m_tail(0)
{
    while (m_capacity < capacity) m_capacity <<= 1;
    m_mask = m_capacity - 1;
    m_buffer = new char[m_capacity];
    std::memset(m_buffer, 0, m_capacity);
}

RingBufferAtomic::~RingBufferAtomic()
{
    delete[] m_buffer;
}

int RingBufferAtomic::write(const QByteArray& data)
{
    int avail = freeSpace();
    int toWrite = qMin(data.size(), avail);
    if (toWrite <= 0) return 0;

    int head = m_head.load(std::memory_order_relaxed);
    for (int i = 0; i < toWrite; ++i) {
        m_buffer[head & m_mask] = data[i];
        head++;
    }
    m_head.store(head, std::memory_order_release);

    m_stats.totalWrites++;
    m_stats.totalBytesWritten += toWrite;
    return toWrite;
}

QByteArray RingBufferAtomic::read(int maxSize)
{
    int avail = available();
    if (avail <= 0) return {};

    int toRead = qMin(maxSize, avail);
    QByteArray result(toRead, '\0');

    int tail = m_tail.load(std::memory_order_relaxed);
    for (int i = 0; i < toRead; ++i) {
        result[i] = m_buffer[tail & m_mask];
        tail++;
    }
    m_tail.store(tail, std::memory_order_release);

    m_stats.totalReads++;
    m_stats.totalBytesRead += toRead;
    return result;
}

bool RingBufferAtomic::writeByte(quint8 byte)
{
    if (isFull()) return false;
    int head = m_head.load(std::memory_order_relaxed);
    m_buffer[head & m_mask] = static_cast<char>(byte);
    m_head.store(head + 1, std::memory_order_release);
    m_stats.totalBytesWritten++;
    return true;
}

bool RingBufferAtomic::readByte(quint8& byte)
{
    if (isEmpty()) return false;
    int tail = m_tail.load(std::memory_order_relaxed);
    byte = static_cast<quint8>(m_buffer[tail & m_mask]);
    m_tail.store(tail + 1, std::memory_order_release);
    m_stats.totalBytesRead++;
    return true;
}

int RingBufferAtomic::available() const
{
    return m_head.load(std::memory_order_acquire) -
           m_tail.load(std::memory_order_acquire);
}

int RingBufferAtomic::freeSpace() const
{
    return m_capacity - available();
}

int RingBufferAtomic::capacity() const { return m_capacity; }
bool RingBufferAtomic::isEmpty() const { return available() == 0; }
bool RingBufferAtomic::isFull() const { return available() >= m_capacity; }

void RingBufferAtomic::clear()
{
    m_head.store(0, std::memory_order_relaxed);
    m_tail.store(0, std::memory_order_relaxed);
}

RingBufferAtomic::Stats RingBufferAtomic::stats() const
{
    return m_stats;
}

void RingBufferAtomic::resetStatistics()
{
    m_stats = Stats{};
}
