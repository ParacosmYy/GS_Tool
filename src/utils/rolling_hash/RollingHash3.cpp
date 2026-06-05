/**
 * @file RollingHash3.cpp
 * @brief 滚动哈希实现 — 可配置基数和模数
 */

#include "utils/rolling_hash/RollingHash3.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param windowSize 窗口大小 @param base 基数 @param modulus 模数 @param parent 父对象 */
RollingHash3::RollingHash3(int windowSize, quint64 base, quint64 modulus, QObject* parent)
    : QObject(parent)
    , m_windowSize(std::max(1, windowSize))
    , m_base(base)
    , m_modulus(modulus)
    , m_hash(0)
    , m_basePow(1)
    , m_count(0)
    , m_initialized(false)
    , m_buffer(m_windowSize, 0)
    , m_head(0)
{
    /* 预计算 base^(windowSize-1) % modulus */
    for (int i = 0; i < m_windowSize - 1; ++i) {
        m_basePow = (m_basePow * m_base) % m_modulus;
    }
}

/** @brief 初始化窗口 @param data 初始数据 @return 是否成功 */
bool RollingHash3::initialize(const QByteArray& data)
{
    if (data.size() < m_windowSize) return false;

    QElapsedTimer timer;
    timer.start();

    m_hash = 0;
    m_count = 0;
    m_head = 0;
    m_buffer.fill(0);

    for (int i = 0; i < m_windowSize; ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        m_hash = (m_hash * m_base + byte) % m_modulus;
        m_buffer[i] = byte;
    }
    m_count = m_windowSize;
    m_head = 0;
    m_initialized = true;

    m_stats.totalBytesProcessed += static_cast<quint64>(m_windowSize);
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalRolls + m_stats.totalHashesComputed + 1);

    return true;
}

/** @brief 滑动窗口 @param newByte 新字节 @return 新哈希值 */
quint64 RollingHash3::roll(char newByte)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized || m_count < m_windowSize) return 0;

    /* 移除最旧字节的影响 */
    quint8 oldByte = m_buffer[m_head];
    quint64 oldContrib = (static_cast<quint64>(oldByte) * m_basePow) % m_modulus;
    if (m_hash >= oldContrib) {
        m_hash = (m_hash - oldContrib) % m_modulus;
    } else {
        m_hash = (m_hash + m_modulus - oldContrib) % m_modulus;
    }

    /* 添加新字节 */
    quint8 nb = static_cast<quint8>(newByte);
    m_hash = (m_hash * m_base + nb) % m_modulus;

    /* 更新缓冲区 */
    m_buffer[m_head] = nb;
    m_head = (m_head + 1) % m_windowSize;

    ++m_stats.totalRolls;
    ++m_stats.totalBytesProcessed;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalRolls + m_stats.totalHashesComputed);

    emit hashRolled(m_hash);
    return m_hash;
}

/** @brief 获取当前哈希 @return 当前哈希值 */
quint64 RollingHash3::currentHash() const
{
    return m_hash;
}

/** @brief 批量计算哈希 @param data 完整数据 @return 哈希列表 */
QVector<quint64> RollingHash3::computeAll(const QByteArray& data)
{
    QVector<quint64> hashes;
    if (data.size() < m_windowSize) return hashes;

    QElapsedTimer timer;
    timer.start();

    hashes.reserve(data.size() - m_windowSize + 1);

    /* 计算第一个窗口 */
    if (!initialize(data)) return hashes;
    hashes.append(m_hash);

    /* 滚动计算后续窗口 */
    for (int i = m_windowSize; i < data.size(); ++i) {
        hashes.append(roll(data[i]));
    }

    m_stats.totalHashesComputed += static_cast<quint64>(hashes.size());
    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalRolls + m_stats.totalHashesComputed);

    emit batchCompleted(hashes.size());
    return hashes;
}

/** @brief 查找所有匹配位置 @param data 数据 @param targetHash 目标哈希 @return 匹配位置 */
QVector<int> RollingHash3::findMatches(const QByteArray& data, quint64 targetHash)
{
    QVector<int> positions;
    if (data.size() < m_windowSize) return positions;

    QElapsedTimer timer;
    timer.start();

    if (!initialize(data)) return positions;

    if (m_hash == targetHash) {
        positions.append(0);
        emit matchFound(0);
    }

    for (int i = m_windowSize; i < data.size(); ++i) {
        roll(data[i]);
        int pos = i - m_windowSize + 1;
        if (m_hash == targetHash) {
            positions.append(pos);
            emit matchFound(pos);
        }
    }

    m_stats.totalHashesComputed += static_cast<quint64>(data.size() - m_windowSize + 1);
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalRolls + m_stats.totalHashesComputed);

    return positions;
}

/** @brief 双哈希(降低碰撞) @param data 数据 @return 双哈希列表 */
QVector<QPair<quint64, quint64>> RollingHash3::computeAllDual(const QByteArray& data)
{
    QVector<QPair<quint64, quint64>> results;
    if (data.size() < m_windowSize) return results;

    QElapsedTimer timer;
    timer.start();

    results.reserve(data.size() - m_windowSize + 1);

    /* 第一哈希: 使用当前参数 */
    quint64 base2 = 131;
    quint64 mod2 = 1000000009;
    quint64 basePow2 = 1;
    for (int i = 0; i < m_windowSize - 1; ++i) {
        basePow2 = (basePow2 * base2) % mod2;
    }

    /* 初始化双哈希 */
    quint64 h1 = 0, h2 = 0;
    for (int i = 0; i < m_windowSize; ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        h1 = (h1 * m_base + byte) % m_modulus;
        h2 = (h2 * base2 + byte) % mod2;
    }
    results.append({h1, h2});

    /* 滚动 */
    for (int i = m_windowSize; i < data.size(); ++i) {
        quint8 oldByte = static_cast<quint8>(data[i - m_windowSize]);
        quint8 newByte = static_cast<quint8>(data[i]);

        /* 哈希1 */
        quint64 old1 = (static_cast<quint64>(oldByte) * m_basePow) % m_modulus;
        h1 = (h1 + m_modulus - old1) % m_modulus;
        h1 = (h1 * m_base + newByte) % m_modulus;

        /* 哈希2 */
        quint64 old2 = (static_cast<quint64>(oldByte) * basePow2) % mod2;
        h2 = (h2 + mod2 - old2) % mod2;
        h2 = (h2 * base2 + newByte) % mod2;

        results.append({h1, h2});
    }

    m_stats.totalHashesComputed += static_cast<quint64>(results.size() * 2);
    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalRolls + m_stats.totalHashesComputed);

    emit batchCompleted(results.size());
    return results;
}

/** @brief 重置统计 */
void RollingHash3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
