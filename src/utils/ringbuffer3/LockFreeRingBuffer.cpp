/**
 * @file LockFreeRingBuffer.cpp
 * @brief 无锁环形缓冲区实现
 */

#include "utils/ringbuffer3/LockFreeRingBuffer.h"

#include <QElapsedTimer>
#include <cstring>

LockFreeRingBuffer::LockFreeRingBuffer(int capacity, QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
    /* 容量取2的幂次以优化取模运算 */
    int pow2 = 1;
    while (pow2 < capacity) pow2 <<= 1;
    m_capacity = pow2;
    m_mask = pow2 - 1;
    m_buffer = new char[m_capacity];
    m_readPos.store(0, std::memory_order_relaxed);
    m_writePos.store(0, std::memory_order_relaxed);
}

LockFreeRingBuffer::~LockFreeRingBuffer()
{
    delete[] m_buffer;
}

int LockFreeRingBuffer::write(const char* data, int len)
{
    int writePos = m_writePos.load(std::memory_order_relaxed);
    int readPos = m_readPos.load(std::memory_order_acquire);
    int avail = m_capacity - (writePos - readPos);

    int toWrite = qMin(len, avail);
    if (toWrite <= 0) {
        m_stats.totalOverflows++;
        emit overflowOccurred(len);
        return 0;
    }

    /* 分段拷贝(环形) */
    int writeIdx = writePos & m_mask;
    int first = qMin(toWrite, m_capacity - writeIdx);
    std::memcpy(m_buffer + writeIdx, data, first);
    if (first < toWrite) {
        std::memcpy(m_buffer, data + first, toWrite - first);
    }

    m_writePos.store(writePos + toWrite, std::memory_order_release);

    m_stats.totalWrites++;
    m_stats.totalBytesWritten += toWrite;
    emit dataWritten(toWrite);
    return toWrite;
}

int LockFreeRingBuffer::read(char* data, int len)
{
    int readPos = m_readPos.load(std::memory_order_relaxed);
    int writePos = m_writePos.load(std::memory_order_acquire);
    int avail = writePos - readPos;

    int toRead = qMin(len, avail);
    if (toRead <= 0) return 0;

    int readIdx = readPos & m_mask;
    int first = qMin(toRead, m_capacity - readIdx);
    std::memcpy(data, m_buffer + readIdx, first);
    if (first < toRead) {
        std::memcpy(data + first, m_buffer, toRead - first);
    }

    m_readPos.store(readPos + toRead, std::memory_order_release);

    m_stats.totalReads++;
    m_stats.totalBytesRead += toRead;
    return toRead;
}

int LockFreeRingBuffer::peek(char* data, int len) const
{
    int readPos = m_readPos.load(std::memory_order_relaxed);
    int writePos = m_writePos.load(std::memory_order_acquire);
    int avail = writePos - readPos;

    int toRead = qMin(len, avail);
    if (toRead <= 0) return 0;

    int readIdx = readPos & m_mask;
    int first = qMin(toRead, m_capacity - readIdx);
    std::memcpy(data, m_buffer + readIdx, first);
    if (first < toRead) {
        std::memcpy(data + first, m_buffer, toRead - first);
    }
    return toRead;
}

int LockFreeRingBuffer::availableRead() const
{
    int readPos = m_readPos.load(std::memory_order_relaxed);
    int writePos = m_writePos.load(std::memory_order_acquire);
    return writePos - readPos;
}

int LockFreeRingBuffer::availableWrite() const
{
    int readPos = m_readPos.load(std::memory_order_acquire);
    int writePos = m_writePos.load(std::memory_order_relaxed);
    return m_capacity - (writePos - readPos);
}

bool LockFreeRingBuffer::isEmpty() const { return availableRead() == 0; }
bool LockFreeRingBuffer::isFull() const { return availableWrite() == 0; }

void LockFreeRingBuffer::clear()
{
    m_readPos.store(0, std::memory_order_relaxed);
    m_writePos.store(0, std::memory_order_relaxed);
}

void LockFreeRingBuffer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
