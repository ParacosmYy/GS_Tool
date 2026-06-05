/**
 * @file WheyHash.cpp
 * @brief WheyHash实现 — 快速非加密哈希算法
 */

#include "utils/hash3/WheyHash.h"

#include <QElapsedTimer>
#include <QtEndian>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
WheyHash::WheyHash(QObject* parent)
    : QObject(parent)
    , m_streamTotal(0)
    , m_streamActive(false)
    , m_timeSum(0.0)
{
    reset();
}

/**
 * @brief 64位循环左移
 * @param v 值
 * @param n 位数
 * @return 左移结果
 */
quint64 WheyHash::rotl64(quint64 v, int n)
{
    return (v << n) | (v >> (64 - n));
}

/**
 * @brief WheyHash核心轮函数
 * @param acc 累加器数组(4个元素)
 * @param block 输入块(32字节)
 *
 * 每轮处理32字节输入，通过混合乘法、旋转和异或
 * 操作更新4个累加器，确保良好的雪崩效应。
 */
void WheyHash::roundFunction(quint64 acc[4],
                               const unsigned char* block) const
{
    /* 读取4个64位小端值 */
    quint64 v0 = qFromLittleEndian<quint64>(block);
    quint64 v1 = qFromLittleEndian<quint64>(block + 8);
    quint64 v2 = qFromLittleEndian<quint64>(block + 16);
    quint64 v3 = qFromLittleEndian<quint64>(block + 24);

    /* WheyHash 常量乘数(素数) */
    const quint64 kMul = 0xbf58476d1ce4e5b9ULL;
    const quint64 kMul2 = 0x94d049bb133111ebULL;

    /* 混合到累加器 */
    acc[0] += v0 * kMul;
    acc[0] = rotl64(acc[0], 31);
    acc[0] *= kMul2;

    acc[1] += v1 * kMul2;
    acc[1] = rotl64(acc[1], 27);
    acc[1] *= kMul;

    acc[2] += v2 * kMul;
    acc[2] = rotl64(acc[2], 33);
    acc[2] *= kMul2;

    acc[3] += v3 * kMul2;
    acc[3] = rotl64(acc[3], 29);
    acc[3] *= kMul;

    /* 交叉混合增加扩散 */
    acc[0] ^= acc[2];
    acc[1] ^= acc[3];
    acc[2] ^= acc[0];
    acc[3] ^= acc[1];
}

/**
 * @brief 处理尾部数据
 * @param acc 累加器数组
 * @param data 尾部数据
 * @param len 数据长度(1~31字节)
 */
void WheyHash::tailProcess(quint64 acc[4],
                             const unsigned char* data, int len) const
{
    const quint64 kMul = 0xbf58476d1ce4e5b9ULL;
    quint64 tail = 0;

    /* 按字节读取尾部数据到单个64位值 */
    int shift = 0;
    for (int i = 0; i < qMin(len, 8); ++i) {
        tail |= static_cast<quint64>(data[i]) << shift;
        shift += 8;
    }

    acc[0] ^= tail;
    acc[0] *= kMul;
    acc[0] = rotl64(acc[0], 13);

    /* 第二组尾部字节(如果有) */
    if (len > 8) {
        quint64 tail2 = 0;
        shift = 0;
        for (int i = 8; i < qMin(len, 16); ++i) {
            tail2 |= static_cast<quint64>(data[i]) << shift;
            shift += 8;
        }
        acc[1] ^= tail2;
        acc[1] *= kMul;
        acc[1] = rotl64(acc[1], 17);
    }

    /* 第三组 */
    if (len > 16) {
        quint64 tail3 = 0;
        shift = 0;
        for (int i = 16; i < qMin(len, 24); ++i) {
            tail3 |= static_cast<quint64>(data[i]) << shift;
            shift += 8;
        }
        acc[2] ^= tail3;
        acc[2] *= kMul;
        acc[2] = rotl64(acc[2], 23);
    }

    /* 第四组 */
    if (len > 24) {
        quint64 tail4 = 0;
        shift = 0;
        for (int i = 24; i < len; ++i) {
            tail4 |= static_cast<quint64>(data[i]) << shift;
            shift += 8;
        }
        acc[3] ^= tail4;
        acc[3] *= kMul;
        acc[3] = rotl64(acc[3], 37);
    }
}

/**
 * @brief 计算单次64位哈希(无种子)
 * @param data 输入数据
 * @return 64位哈希值
 */
quint64 WheyHash::hash(const QByteArray& data) const
{
    return hash64(data, 0);
}

/**
 * @brief 带种子的64位哈希
 * @param data 输入数据
 * @param seed 种子值
 * @return 64位哈希值
 */
quint64 WheyHash::hash64(const QByteArray& data, quint64 seed) const
{
    QElapsedTimer timer;
    timer.start();

    int len = data.size();
    const unsigned char* ptr =
        reinterpret_cast<const unsigned char*>(data.constData());

    /* 初始化累加器(基于种子) */
    quint64 acc[4] = {
        seed ^ 0x243f6a8885a308d3ULL,
        seed ^ 0x13198a2e03707344ULL,
        seed ^ 0xa4093822299f31d0ULL,
        seed ^ 0x082efa98ec4e6c89ULL
    };

    /* 处理完整的32字节块 */
    int numBlocks = len / 32;
    for (int i = 0; i < numBlocks; ++i) {
        roundFunction(acc, ptr + i * 32);
    }

    /* 处理尾部 */
    int tailLen = len % 32;
    if (tailLen > 0) {
        tailProcess(acc, ptr + numBlocks * 32, tailLen);
    }

    /* 最终混合(avalanche) */
    acc[0] ^= static_cast<quint64>(len);
    acc[1] ^= static_cast<quint64>(len) * 0x9e3779b97f4a7c15ULL;

    acc[0] ^= acc[0] >> 33;
    acc[0] *= 0xff51afd7ed558ccdULL;
    acc[0] ^= acc[0] >> 33;

    acc[1] ^= acc[1] >> 33;
    acc[1] *= 0xc4ceb9fe1a85ec53ULL;
    acc[1] ^= acc[1] >> 33;

    /* 合并4个累加器为最终64位输出 */
    quint64 result = acc[0] ^ acc[1] ^ acc[2] ^ acc[3];
    result ^= result >> 33;
    result *= 0xff51afd7ed558ccdULL;
    result ^= result >> 33;

    /* 更新统计(线程安全只读操作，const_cast绕过) */
    const_cast<WheyHash*>(this)->m_stats.totalHashed++;
    const_cast<WheyHash*>(this)->m_stats.totalBytes +=
        static_cast<quint64>(len);
    double elapsed = static_cast<double>(timer.elapsed());
    const_cast<WheyHash*>(this)->m_timeSum += elapsed;
    const_cast<WheyHash*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalHashed);

    const_cast<WheyHash*>(this)->emit hashComputed(result,
        static_cast<quint64>(len));
    return result;
}

/**
 * @brief 计算128位哈希
 * @param data 输入数据
 * @return QPair(高64位, 低64位)
 */
QPair<quint64, quint64> WheyHash::hash128(const QByteArray& data) const
{
    int len = data.size();
    const unsigned char* ptr =
        reinterpret_cast<const unsigned char*>(data.constData());

    quint64 acc[4] = {
        0x243f6a8885a308d3ULL,
        0x13198a2e03707344ULL,
        0xa4093822299f31d0ULL,
        0x082efa98ec4e6c89ULL
    };

    int numBlocks = len / 32;
    for (int i = 0; i < numBlocks; ++i) {
        roundFunction(acc, ptr + i * 32);
    }

    int tailLen = len % 32;
    if (tailLen > 0) {
        tailProcess(acc, ptr + numBlocks * 32, tailLen);
    }

    /* 最终混合 — 输出两个独立的64位值 */
    acc[0] ^= static_cast<quint64>(len);
    acc[1] ^= static_cast<quint64>(len) * 0x9e3779b97f4a7c15ULL;
    acc[2] ^= static_cast<quint64>(len) * 0x517cc1b727220a95ULL;
    acc[3] ^= static_cast<quint64>(len);

    /* 独立avalanche高半部分和低半部分 */
    quint64 hi = acc[0] ^ acc[2];
    hi ^= hi >> 33;
    hi *= 0xff51afd7ed558ccdULL;
    hi ^= hi >> 33;
    hi *= 0xc4ceb9fe1a85ec53ULL;
    hi ^= hi >> 33;

    quint64 lo = acc[1] ^ acc[3];
    lo ^= lo >> 33;
    lo *= 0xff51afd7ed558ccdULL;
    lo ^= lo >> 33;
    lo *= 0xc4ceb9fe1a85ec53ULL;
    lo ^= lo >> 33;

    return {hi, lo};
}

/**
 * @brief 流式更新 — 追加数据到内部状态
 * @param data 追加的数据块
 */
void WheyHash::update(const QByteArray& data)
{
    if (data.isEmpty()) return;

    m_streamActive = true;
    m_streamTotal += static_cast<quint64>(data.size());

    /* 将新数据追加到缓冲区 */
    m_buffer.append(data);

    /* 处理缓冲区中完整的32字节块 */
    const unsigned char* ptr =
        reinterpret_cast<const unsigned char*>(m_buffer.constData());
    int numBlocks = m_buffer.size() / 32;

    for (int i = 0; i < numBlocks; ++i) {
        roundFunction(m_streamState, ptr + i * 32);
    }

    /* 保留未处理的尾部 */
    int consumed = numBlocks * 32;
    if (consumed > 0) {
        m_buffer = m_buffer.mid(consumed);
    }
}

/**
 * @brief 完成流式哈希计算
 * @return 最终64位哈希值
 */
quint64 WheyHash::finalize()
{
    if (!m_streamActive) return 0;

    QElapsedTimer timer;
    timer.start();

    /* 处理缓冲区中剩余的尾部数据 */
    if (!m_buffer.isEmpty()) {
        const unsigned char* ptr =
            reinterpret_cast<const unsigned char*>(m_buffer.constData());
        tailProcess(m_streamState, ptr, m_buffer.size());
    }

    /* 最终混合 */
    m_streamState[0] ^= m_streamTotal;
    m_streamState[1] ^= m_streamTotal * 0x9e3779b97f4a7c15ULL;

    m_streamState[0] ^= m_streamState[0] >> 33;
    m_streamState[0] *= 0xff51afd7ed558ccdULL;
    m_streamState[0] ^= m_streamState[0] >> 33;

    m_streamState[1] ^= m_streamState[1] >> 33;
    m_streamState[1] *= 0xc4ceb9fe1a85ec53ULL;
    m_streamState[1] ^= m_streamState[1] >> 33;

    quint64 result = m_streamState[0] ^ m_streamState[1]
        ^ m_streamState[2] ^ m_streamState[3];
    result ^= result >> 33;
    result *= 0xff51afd7ed558ccdULL;
    result ^= result >> 33;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalHashed;
    m_stats.totalBytes += m_streamTotal;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalHashed);

    emit hashComputed(result, m_streamTotal);

    /* 重置流式状态以备下次使用 */
    reset();
    return result;
}

/**
 * @brief 重置流式状态
 */
void WheyHash::reset()
{
    m_streamState[0] = 0x243f6a8885a308d3ULL;
    m_streamState[1] = 0x13198a2e03707344ULL;
    m_streamState[2] = 0xa4093822299f31d0ULL;
    m_streamState[3] = 0x082efa98ec4e6c89ULL;
    m_buffer.clear();
    m_streamTotal = 0;
    m_streamActive = false;
}

/** @brief 重置统计 */
void WheyHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
