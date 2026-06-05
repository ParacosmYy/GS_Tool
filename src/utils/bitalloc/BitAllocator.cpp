/**
 * @file BitAllocator.cpp
 * @brief 位级内存分配器实现
 */

#include "utils/bitalloc/BitAllocator.h"

#include <QtMath>
#include <algorithm>

BitAllocator::BitAllocator(int totalBits, QObject* parent)
    : QObject(parent)
    , m_totalBits(qMax(1, totalBits))
    , m_strategy(Strategy::FirstFit)
    , m_usedBits(0)
{
    m_pool.resize(m_totalBits, false);
}

void BitAllocator::setStrategy(Strategy strategy)
{
    m_strategy = strategy;
}

BitAllocator::AllocHandle BitAllocator::allocate(int bitCount)
{
    AllocHandle handle;
    if (bitCount <= 0 || bitCount > m_totalBits) return handle;

    /* 收集空闲块 */
    QVector<FreeBlock> freeList = freeBlocks();

    int bestStart = -1;
    int bestSize = m_totalBits + 1; // 用于BestFit

    switch (m_strategy) {
    case Strategy::FirstFit:
        for (const FreeBlock& block : freeList) {
            if (block.bitCount >= bitCount) {
                bestStart = block.startBit;
                bestSize = block.bitCount;
                break; // 首次适配，找到即停
            }
        }
        break;

    case Strategy::BestFit:
        for (const FreeBlock& block : freeList) {
            if (block.bitCount >= bitCount && block.bitCount < bestSize) {
                bestStart = block.startBit;
                bestSize = block.bitCount;
            }
        }
        break;
    }

    if (bestStart < 0) return handle; // 无合适空闲块

    /* 标记已分配 */
    for (int i = bestStart; i < bestStart + bitCount; ++i) {
        m_pool[i] = true;
    }

    handle.startBit = bestStart;
    handle.bitCount = bitCount;
    handle.valid = true;

    m_allocations[bestStart] = bitCount;
    m_usedBits += bitCount;

    /* 更新统计 */
    ++m_stats.totalAllocs;
    if (static_cast<quint64>(m_usedBits) > m_stats.peakUsage) {
        m_stats.peakUsage = static_cast<quint64>(m_usedBits);
    }
    recalculateFragmentation();

    emit allocated(handle);
    return handle;
}

void BitAllocator::deallocate(const AllocHandle& handle)
{
    if (!handle.valid) return;

    /* 验证该分配是否存在 */
    if (!m_allocations.contains(handle.startBit)) return;
    if (m_allocations[handle.startBit] != handle.bitCount) return;

    /* 清除标记 */
    for (int i = handle.startBit; i < handle.startBit + handle.bitCount; ++i) {
        m_pool[i] = false;
    }

    m_allocations.remove(handle.startBit);
    m_usedBits -= handle.bitCount;

    /* 更新统计 */
    ++m_stats.totalFrees;
    recalculateFragmentation();

    emit deallocated(handle.startBit, handle.bitCount);
}

QVector<BitAllocator::FreeBlock> BitAllocator::freeBlocks() const
{
    QVector<FreeBlock> blocks;
    int i = 0;
    while (i < m_totalBits) {
        if (!m_pool[i]) {
            int start = i;
            int count = 0;
            while (i < m_totalBits && !m_pool[i]) {
                ++count;
                ++i;
            }
            blocks.append({start, count});
        } else {
            ++i;
        }
    }
    return blocks;
}

QVector<BitAllocator::AllocHandle> BitAllocator::allocatedBlocks() const
{
    QVector<AllocHandle> blocks;
    for (auto it = m_allocations.begin(); it != m_allocations.end(); ++it) {
        blocks.append({it.key(), it.value(), true});
    }
    /* 按起始位排序 */
    std::sort(blocks.begin(), blocks.end(),
        [](const AllocHandle& a, const AllocHandle& b) {
            return a.startBit < b.startBit;
        });
    return blocks;
}

int BitAllocator::usedBits() const
{
    return m_usedBits;
}

int BitAllocator::freeBits() const
{
    return m_totalBits - m_usedBits;
}

int BitAllocator::totalBits() const
{
    return m_totalBits;
}

void BitAllocator::reset()
{
    std::fill(m_pool.begin(), m_pool.end(), false);
    m_allocations.clear();
    m_usedBits = 0;
    m_stats.fragmentationRatio = 0.0;
}

void BitAllocator::recalculateFragmentation()
{
    QVector<FreeBlock> freeList = freeBlocks();

    if (freeList.isEmpty() || m_usedBits == 0) {
        m_stats.fragmentationRatio = 0.0;
        return;
    }

    /* 碎片率 = 1 - (最大空闲块 / 总空闲空间)
       全在一个连续块时碎片率为0，越分散碎片率越高 */
    int totalFree = m_totalBits - m_usedBits;
    int maxFreeBlock = 0;
    for (const FreeBlock& block : freeList) {
        if (block.bitCount > maxFreeBlock) {
            maxFreeBlock = block.bitCount;
        }
    }

    if (totalFree > 0) {
        m_stats.fragmentationRatio = 1.0 -
            static_cast<double>(maxFreeBlock) / static_cast<double>(totalFree);
    } else {
        m_stats.fragmentationRatio = 0.0;
    }

    /* 高碎片率告警 */
    if (m_stats.fragmentationRatio > 0.75) {
        emit fragmentationWarning(m_stats.fragmentationRatio);
    }
}

void BitAllocator::resetStatistics()
{
    m_stats = Stats{};
}
