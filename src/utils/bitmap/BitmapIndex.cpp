/**
 * @file BitmapIndex.cpp
 * @brief 位图索引引擎实现
 */

#include "BitmapIndex.h"

#include <algorithm>
#include <QtGlobal>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

BitmapIndex::BitmapIndex(qsizetype size, QObject* parent)
    : QObject(parent)
    , m_size(((size + 63) / 64) * 64)  // 对齐到64位
    , m_data(((m_size + 7) / 8), 0)
{
}

BitmapIndex::~BitmapIndex() = default;

// ═══════════════════════════════════════════════════════════
// 位操作
// ═══════════════════════════════════════════════════════════

void BitmapIndex::setBit(qsizetype pos)
{
    if (pos < 0) return;
    ensureCapacity(pos);
    m_data[static_cast<int>(pos / 8)] |= static_cast<char>(1U << (7 - (pos % 8)));
    m_stats.totalBitOps++;
    m_stats.totalSetBits++;
}

void BitmapIndex::clearBit(qsizetype pos)
{
    if (pos < 0 || pos >= m_size) return;
    m_data[static_cast<int>(pos / 8)] &= static_cast<char>(~(1U << (7 - (pos % 8))));
    m_stats.totalBitOps++;
    m_stats.totalClearBits++;
}

bool BitmapIndex::testBit(qsizetype pos) const
{
    if (pos < 0 || pos >= m_size) return false;
    return (static_cast<quint8>(m_data[static_cast<int>(pos / 8)]) &
            (1U << (7 - (pos % 8)))) != 0;
}

void BitmapIndex::toggleBit(qsizetype pos)
{
    if (pos < 0) return;
    ensureCapacity(pos);
    m_data[static_cast<int>(pos / 8)] ^= static_cast<char>(1U << (7 - (pos % 8)));
    m_stats.totalBitOps++;
}

void BitmapIndex::setRange(qsizetype begin, qsizetype end)
{
    for (qsizetype i = begin; i < end && i < m_size; ++i) {
        setBit(i);
    }
    m_stats.totalOperations++;
}

void BitmapIndex::clearRange(qsizetype begin, qsizetype end)
{
    for (qsizetype i = begin; i < end && i < m_size; ++i) {
        clearBit(i);
    }
    m_stats.totalOperations++;
}

// ═══════════════════════════════════════════════════════════
// 集合运算
// ═══════════════════════════════════════════════════════════

void BitmapIndex::bitwiseAnd(const BitmapIndex& other)
{
    const int len = std::min(m_data.size(), other.m_data.size());
    for (int i = 0; i < len; ++i) {
        m_data[i] = m_data[i] & other.m_data[i];
    }
    for (int i = len; i < m_data.size(); ++i) {
        m_data[i] = 0;
    }
    m_stats.totalOperations++;
    emit operationCompleted(QStringLiteral("AND"), popcount());
}

void BitmapIndex::bitwiseOr(const BitmapIndex& other)
{
    const int len = std::min(m_data.size(), other.m_data.size());
    for (int i = 0; i < len; ++i) {
        m_data[i] = m_data[i] | other.m_data[i];
    }
    m_stats.totalOperations++;
    emit operationCompleted(QStringLiteral("OR"), popcount());
}

void BitmapIndex::bitwiseXor(const BitmapIndex& other)
{
    const int len = std::min(m_data.size(), other.m_data.size());
    for (int i = 0; i < len; ++i) {
        m_data[i] = m_data[i] ^ other.m_data[i];
    }
    m_stats.totalOperations++;
    emit operationCompleted(QStringLiteral("XOR"), popcount());
}

void BitmapIndex::bitwiseNot()
{
    for (int i = 0; i < m_data.size(); ++i) {
        m_data[i] = static_cast<char>(~static_cast<quint8>(m_data[i]));
    }
    m_stats.totalOperations++;
    emit operationCompleted(QStringLiteral("NOT"), popcount());
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

qsizetype BitmapIndex::popcount() const
{
    qsizetype count = 0;
    // 处理完整的quint64块
    const int numQwords = m_data.size() / 8;
    for (int i = 0; i < numQwords; ++i) {
        quint64 v = 0;
        for (int j = 0; j < 8; ++j) {
            v = (v << 8) | static_cast<quint8>(m_data[i * 8 + j]);
        }
        count += popcount64(v);
    }
    // 处理剩余字节
    for (int i = numQwords * 8; i < m_data.size(); ++i) {
        count += popcount64(static_cast<quint64>(static_cast<quint8>(m_data[i])));
    }
    return count;
}

qsizetype BitmapIndex::findFirst() const
{
    return findNext(0);
}

qsizetype BitmapIndex::findNext(qsizetype pos) const
{
    for (qsizetype i = pos; i < m_size; ++i) {
        if (testBit(i)) return i;
    }
    return -1;
}

qsizetype BitmapIndex::size() const { return m_size; }
qsizetype BitmapIndex::byteSize() const { return m_data.size(); }

bool BitmapIndex::isEmpty() const
{
    return popcount() == 0;
}

// ═══════════════════════════════════════════════════════════
// 序列化
// ═══════════════════════════════════════════════════════════

QByteArray BitmapIndex::toByteArray() const { return m_data; }

void BitmapIndex::fromByteArray(const QByteArray& data, qsizetype bitCount)
{
    m_size = ((bitCount + 63) / 64) * 64;
    m_data = data;
    m_data.resize(static_cast<int>((m_size + 7) / 8));
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

BitmapIndex::Stats BitmapIndex::stats() const { return m_stats; }

void BitmapIndex::resetStatistics() { m_stats = Stats{}; }

void BitmapIndex::clear()
{
    m_data.fill(0);
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

void BitmapIndex::ensureCapacity(qsizetype pos)
{
    if (pos >= m_size) {
        qsizetype newSize = ((pos + 64) / 64) * 64;
        m_data.resize(static_cast<int>((newSize + 7) / 8), 0);
        m_size = newSize;
    }
}

int BitmapIndex::popcount64(quint64 v)
{
    // 标准位计数算法
    v = v - ((v >> 1) & 0x5555555555555555ULL);
    v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
    v = (v + (v >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return static_cast<int>((v * 0x0101010101010101ULL) >> 56);
}
