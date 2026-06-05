/**
 * @file MPSCRingBuffer.cpp
 * @brief 多生产者单消费者无锁环形缓冲区实现
 */

#include "utils/ring5/MPSCRingBuffer.h"

#include <QElapsedTimer>
#include <algorithm>

MPSCRingBuffer::MPSCRingBuffer(int capacity, QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_batchSum(0)
{
    /* 容量向上取整到2的幂, 方便取模运算 */
    int pow2 = 1;
    while (pow2 < qMax(16, capacity)) pow2 <<= 1;
    m_capacity = pow2;
    m_mask = pow2 - 1;

    m_buffer.resize(m_capacity);
    for (auto& slot : m_buffer) {
        slot.state.store(static_cast<int>(SlotState::Free));
    }

    m_writeHead.store(0);
    m_writeTail.store(0);
    m_readHead.store(0);
    m_committed.store(0);
}

MPSCRingBuffer::~MPSCRingBuffer() = default;

bool MPSCRingBuffer::write(double data)
{
    QElapsedTimer timer;
    timer.start();

    int slot = acquireWriteSlot();
    if (slot < 0) {
        ++m_stats.totalOverflows;
        emit overflow(1);
        return false;
    }

    /* 写入数据 */
    m_buffer[slot].data = data;
    commitWriteSlot(slot);

    /* 更新统计 */
    ++m_stats.totalItemsWritten;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalItemsWritten + m_stats.totalItemsRead;
    if (totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
    }

    return true;
}

int MPSCRingBuffer::writeBatch(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return 0;

    int written = 0;
    for (int i = 0; i < data.size(); ++i) {
        int slot = acquireWriteSlot();
        if (slot < 0) {
            m_stats.totalOverflows++;
            if (written > 0) emit overflow(data.size() - written);
            break;
        }
        m_buffer[slot].data = data[i];
        commitWriteSlot(slot);
        ++written;
    }

    /* 更新统计 */
    m_stats.totalItemsWritten += written;
    ++m_stats.totalWrites;

    /* 更新批量大小平均 */
    m_batchSum += written;
    m_stats.avgBatchSize = static_cast<double>(m_batchSum) /
                           m_stats.totalWrites;

    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalItemsWritten + m_stats.totalItemsRead;
    if (totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
    }

    if (written > 0) emit batchWritten(written);
    return written;
}

bool MPSCRingBuffer::read(double& value)
{
    /* 消费者端: 单线程, 无需原子操作 */
    int committed = m_committed.loadAcquire();
    int readPos = m_readHead.loadAcquire();

    if (readPos >= committed) {
        ++m_stats.totalReadUnderruns;
        return false;
    }

    int slot = readPos & m_mask;
    int state = m_buffer[slot].state.loadAcquire();

    /* 等待数据就绪(自旋等待, 生产者很快会提交) */
    int spins = 0;
    while (state != static_cast<int>(SlotState::Ready) && spins < 1000) {
        state = m_buffer[slot].state.loadAcquire();
        ++spins;
    }

    if (state != static_cast<int>(SlotState::Ready)) {
        ++m_stats.totalReadUnderruns;
        return false;
    }

    /* 读取数据 */
    m_buffer[slot].state.store(static_cast<int>(SlotState::Reading));
    value = m_buffer[slot].data;
    m_buffer[slot].state.store(static_cast<int>(SlotState::Free));

    m_readHead.storeRelease(readPos + 1);

    /* 更新统计 */
    ++m_stats.totalItemsRead;
    int totalOps = m_stats.totalItemsWritten + m_stats.totalItemsRead;
    if (totalOps > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
    }

    return true;
}

QVector<double> MPSCRingBuffer::readBatch(int maxItems)
{
    QVector<double> result;
    if (maxItems <= 0) return result;

    int committed = m_committed.loadAcquire();
    int readPos = m_readHead.loadAcquire();
    int available = committed - readPos;
    int toRead = qMin(maxItems, available);

    result.reserve(toRead);

    for (int i = 0; i < toRead; ++i) {
        int slot = (readPos + i) & m_mask;
        int state = m_buffer[slot].state.loadAcquire();

        if (state != static_cast<int>(SlotState::Ready)) break;

        m_buffer[slot].state.store(static_cast<int>(SlotState::Reading));
        result.append(m_buffer[slot].data);
        m_buffer[slot].state.store(static_cast<int>(SlotState::Free));
    }

    m_readHead.storeRelease(readPos + result.size());
    m_stats.totalItemsRead += result.size();

    return result;
}

int MPSCRingBuffer::available() const
{
    int committed = m_committed.loadAcquire();
    int readPos = m_readHead.loadAcquire();
    return qMax(0, committed - readPos);
}

int MPSCRingBuffer::freeSpace() const
{
    return m_capacity - available();
}

bool MPSCRingBuffer::isEmpty() const
{
    return available() == 0;
}

void MPSCRingBuffer::reset()
{
    for (auto& slot : m_buffer) {
        slot.state.store(static_cast<int>(SlotState::Free));
    }
    m_writeHead.store(0);
    m_writeTail.store(0);
    m_readHead.store(0);
    m_committed.store(0);
}

int MPSCRingBuffer::capacity() const
{
    return m_capacity;
}

MPSCRingBuffer::Stats MPSCRingBuffer::stats() const
{
    return m_stats;
}

void MPSCRingBuffer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_batchSum = 0;
}

int MPSCRingBuffer::acquireWriteSlot()
{
    /* CAS循环: 尝试获取写入位置 */
    int retries = 0;
    const int maxRetries = 100;

    while (retries < maxRetries) {
        int head = m_writeHead.loadAcquire();

        /* 检查是否有空间 */
        int committed = m_committed.loadAcquire();
        int readPos = m_readHead.loadAcquire();
        if (head - readPos >= m_capacity) {
            return -1;  /* 缓冲区满 */
        }

        /* CAS获取写入位置 */
        if (m_writeHead.testAndSetOrdered(head, head + 1)) {
            int slot = head & m_mask;

            /* 等待槽位空闲 */
            int spins = 0;
            while (m_buffer[slot].state.loadAcquire() !=
                   static_cast<int>(SlotState::Free) && spins < 1000) {
                ++spins;
            }

            if (m_buffer[slot].state.loadAcquire() !=
                static_cast<int>(SlotState::Free)) {
                return -1;
            }

            m_buffer[slot].state.storeRelease(
                static_cast<int>(SlotState::Writing));
            return slot;
        }

        ++retries;
        ++m_stats.totalWriteRetries;
    }

    return -1;
}

void MPSCRingBuffer::commitWriteSlot(int slot)
{
    m_buffer[slot].state.storeRelease(
        static_cast<int>(SlotState::Ready));

    /* 更新已提交计数 */
    m_committed.fetchAndAddOrdered(1);
}
