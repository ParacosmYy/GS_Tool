/**
 * @file HyperLogLog.cpp
 * @brief HyperLogLog基数估计器实现
 */

#include "HyperLogLog.h"

#include <QElapsedTimer>
#include <algorithm>
#include <QtGlobal>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

HyperLogLog::HyperLogLog(int precision, QObject* parent)
    : QObject(parent)
    , m_precision(std::min({std::max(precision, 4), 18}))
    , m_registerCount(1 << m_precision)
    , m_registers(m_registerCount, 0)
    , m_isDense(false)
    , m_sparseLimit(static_cast<quint64>(m_registerCount / 4))
{
}

HyperLogLog::~HyperLogLog() = default;

// ═══════════════════════════════════════════════════════════
// 数据操作
// ═══════════════════════════════════════════════════════════

void HyperLogLog::add(const QByteArray& data)
{
    const quint32 hash = murmurHash3(data, 0x12345678);
    addInt(hash);
}

void HyperLogLog::addInt(quint32 value)
{
    // 使用低p位作为寄存器索引
    const int idx = static_cast<int>(value & static_cast<quint32>((1 << m_precision) - 1));
    // 剩余位计算前导零个数
    const quint32 w = value >> m_precision;
    const int rho = HyperLogLog::rho(w, 32 - m_precision);

    if (!m_isDense) {
        // 稀疏模式
        const quint32 encoded = static_cast<quint32>(idx) << 8 | static_cast<quint32>(rho);

        // 检查是否已有更大的值
        bool updated = false;
        for (auto it = m_sparseSet.begin(); it != m_sparseSet.end(); ++it) {
            const int existingIdx = static_cast<int>((*it >> 8) & 0xFF);
            if (existingIdx == idx) {
                const int existingRho = static_cast<int>(*it & 0xFF);
                if (rho > existingRho) {
                    m_sparseSet.erase(it);
                    m_sparseSet.insert(encoded);
                }
                updated = true;
                break;
            }
        }
        if (!updated) {
            m_sparseSet.insert(encoded);
        }

        if (static_cast<quint64>(m_sparseSet.size()) >= m_sparseLimit) {
            promoteToDense();
        }
    } else {
        // 密集模式
        if (rho > m_registers[idx]) {
            m_registers[idx] = static_cast<quint8>(rho);
        }
    }

    m_stats.totalAdds++;

    // 定期更新基数估计
    if (m_stats.totalAdds % 1000 == 0) {
        m_stats.lastEstimate = cardinality();
    }
}

void HyperLogLog::merge(const HyperLogLog& other)
{
    if (other.m_precision != m_precision) {
        emit error(tr("HLL合并错误: 精度参数不匹配"));
        return;
    }

    // 确保两者都是密集模式
    if (!m_isDense) promoteToDense();
    if (!other.m_isDense) {
        // 将other的稀疏数据合并到寄存器
        for (const quint32& encoded : other.m_sparseSet) {
            const int idx = static_cast<int>((encoded >> 8) & 0xFF);
            const int rho = static_cast<int>(encoded & 0xFF);
            if (idx < m_registerCount && rho > m_registers[idx]) {
                m_registers[idx] = static_cast<quint8>(rho);
            }
        }
    } else {
        for (int i = 0; i < m_registerCount; ++i) {
            if (other.m_registers[i] > m_registers[i]) {
                m_registers[i] = other.m_registers[i];
            }
        }
    }

    m_stats.totalMerges++;
    m_stats.lastEstimate = cardinality();
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

double HyperLogLog::cardinality()
{
    if (!m_isDense) promoteToDense();

    const double estimate = rawEstimate();

    // 小基数修正
    double result = estimate;
    const int m = m_registerCount;

    if (estimate <= 2.5 * static_cast<double>(m)) {
        // 线性计数修正
        int zeros = 0;
        for (int i = 0; i < m; ++i) {
            if (m_registers[i] == 0) zeros++;
        }
        if (zeros > 0) {
            const double lc = alpha(m) * static_cast<double>(m) * static_cast<double>(m) /
                              static_cast<double>(zeros);
            result = lc;
        }
    }
    // 大基数修正
    else if (estimate > (1.0 / 30.0) * std::pow(2.0, 32)) {
        result = -std::pow(2.0, 32) * std::log10(1.0 - estimate / std::pow(2.0, 32)) /
                 std::log10(2.0);
    }

    m_stats.totalQueries++;
    m_stats.lastEstimate = result;
    emit cardinalityUpdated(result);
    return result;
}

bool HyperLogLog::isEmpty() const
{
    if (m_isDense) {
        for (int i = 0; i < m_registerCount; ++i) {
            if (m_registers[i] > 0) return false;
        }
        return true;
    }
    return m_sparseSet.isEmpty();
}

// ═══════════════════════════════════════════════════════════
// 序列化
// ═══════════════════════════════════════════════════════════

QByteArray HyperLogLog::serialize() const
{
    QByteArray result;
    result.append(static_cast<char>(m_precision));
    result.append(static_cast<char>(m_isDense ? 1 : 0));

    if (m_isDense) {
        for (int i = 0; i < m_registerCount; ++i) {
            result.append(static_cast<char>(m_registers[i]));
        }
    } else {
        // 稀疏模式: 写入非零寄存器数 + (index, value)对
        const int count = m_sparseSet.size();
        result.append(static_cast<char>((count >> 8) & 0xFF));
        result.append(static_cast<char>(count & 0xFF));
        for (const quint32& encoded : m_sparseSet) {
            result.append(static_cast<char>((encoded >> 8) & 0xFF));
            result.append(static_cast<char>(encoded & 0xFF));
        }
    }
    return result;
}

bool HyperLogLog::deserialize(const QByteArray& data)
{
    if (data.size() < 2) return false;

    m_precision = static_cast<int>(static_cast<quint8>(data[0]));
    m_registerCount = 1 << m_precision;
    m_registers.resize(m_registerCount);
    m_registers.fill(0);
    m_sparseSet.clear();

    m_isDense = static_cast<quint8>(data[1]) != 0;

    if (m_isDense) {
        if (data.size() < 2 + m_registerCount) return false;
        for (int i = 0; i < m_registerCount; ++i) {
            m_registers[i] = static_cast<quint8>(data[2 + i]);
        }
    } else {
        if (data.size() < 4) return false;
        const int count = (static_cast<quint8>(data[2]) << 8) |
                          static_cast<quint8>(data[3]);
        int pos = 4;
        for (int i = 0; i < count && pos + 1 < data.size(); ++i) {
            quint32 encoded = (static_cast<quint32>(static_cast<quint8>(data[pos])) << 8) |
                              static_cast<quint32>(static_cast<quint8>(data[pos + 1]));
            m_sparseSet.insert(encoded);
            pos += 2;
        }
    }
    return true;
}

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

int HyperLogLog::precision() const { return m_precision; }
int HyperLogLog::registerCount() const { return m_registerCount; }

void HyperLogLog::reset()
{
    m_registers.fill(0);
    m_sparseSet.clear();
    m_isDense = false;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

HyperLogLog::Stats HyperLogLog::stats() const { return m_stats; }
void HyperLogLog::resetStatistics() { m_stats = Stats{}; }

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

quint32 HyperLogLog::murmurHash3(const QByteArray& data, quint32 seed)
{
    quint32 h = seed;
    const int len = data.size();
    const int nblocks = len / 4;

    for (int i = 0; i < nblocks; ++i) {
        quint32 k = static_cast<quint32>(static_cast<quint8>(data[i * 4])) |
                    (static_cast<quint32>(static_cast<quint8>(data[i * 4 + 1])) << 8) |
                    (static_cast<quint32>(static_cast<quint8>(data[i * 4 + 2])) << 16) |
                    (static_cast<quint32>(static_cast<quint8>(data[i * 4 + 3])) << 24);

        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;

        h ^= k;
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }

    // 处理尾部
    quint32 k1 = 0;
    const int tail = nblocks * 4;
    switch (len & 3) {
    case 3: k1 ^= static_cast<quint32>(static_cast<quint8>(data[tail + 2])) << 16;
        // fall through
    case 2: k1 ^= static_cast<quint32>(static_cast<quint8>(data[tail + 1])) << 8;
        // fall through
    case 1: k1 ^= static_cast<quint32>(static_cast<quint8>(data[tail]));
        k1 *= 0xcc9e2d51;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= 0x1b873593;
        h ^= k1;
    }

    h ^= static_cast<quint32>(len);
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

int HyperLogLog::rho(quint32 w, int bits)
{
    if (w == 0) return bits + 1;
    int count = 1;
    while ((w & 1) == 0) {
        count++;
        w >>= 1;
    }
    return count;
}

void HyperLogLog::promoteToDense()
{
    if (m_isDense) return;

    for (const quint32& encoded : m_sparseSet) {
        const int idx = static_cast<int>((encoded >> 8) & 0xFF);
        const int rho = static_cast<int>(encoded & 0xFF);
        if (idx < m_registerCount && rho > m_registers[idx]) {
            m_registers[idx] = static_cast<quint8>(rho);
        }
    }
    m_sparseSet.clear();
    m_isDense = true;
}

double HyperLogLog::rawEstimate() const
{
    double sum = 0.0;
    for (int i = 0; i < m_registerCount; ++i) {
        sum += 1.0 / static_cast<double>(1ULL << m_registers[i]);
    }
    return alpha(m_registerCount) * static_cast<double>(m_registerCount) *
           static_cast<double>(m_registerCount) / sum;
}

double HyperLogLog::alpha(int m) const
{
    switch (m) {
    case 16: return 0.673;
    case 32: return 0.697;
    case 64: return 0.709;
    default: return 0.7213 / (1.0 + 1.079 / static_cast<double>(m));
    }
}
