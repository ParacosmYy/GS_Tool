/**
 * @file XxHash.cpp
 * @brief xxHash64 快速非加密哈希算法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/xxhash/XxHash.h"

/** @brief 构造函数 @param parent 父对象 */
XxHash::XxHash(QObject* parent)
    : QObject(parent)
    , m_seed(0)
    , m_finalized(false)
    , m_timeSum(0.0)
{
    memset(m_acc, 0, sizeof(m_acc));
}

/** @brief 初始化哈希状态 @param seed 种子值 */
void XxHash::init(quint64 seed)
{
    m_seed = seed;
    m_finalized = false;
    m_buffer.clear();

    /* 初始化4个累加器 */
    m_acc[0] = seed + PRIME64_1 + PRIME64_2;
    m_acc[1] = seed + PRIME64_2;
    m_acc[2] = seed;
    m_acc[3] = seed - PRIME64_1;
}

/** @brief 追加数据到哈希计算 @param data 输入数据 */
void XxHash::update(const QByteArray& data)
{
    if (data.isEmpty() || m_finalized) return;

    m_buffer.append(data);
    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());
    ++m_stats.totalUpdates;

    /* 处理所有完整的 stripe(32字节) */
    while (m_buffer.size() >= STRIPE_LEN) {
        processStripe(m_buffer.constData());
        m_buffer.remove(0, STRIPE_LEN);
    }
}

/** @brief 处理一个 stripe(32字节) @param data 数据指针 */
void XxHash::processStripe(const char* data)
{
    for (int lane = 0; lane < 4; ++lane) {
        quint64 val = 0;
        for (int b = 0; b < 8; ++b) {
            val |= static_cast<quint64>(static_cast<quint8>(data[lane * 8 + b]))
                   << (8 * b);
        }

        /* stripe 累加: acc = acc + (val * PRIME64_2) */
        m_acc[lane] += val * PRIME64_2;
        /* 左旋31位 */
        m_acc[lane] = (m_acc[lane] << 31) | (m_acc[lane] >> 33);
        m_acc[lane] *= PRIME64_1;
    }
}

/** @brief 完成哈希计算并返回64位摘要 @return 64位哈希值 */
quint64 XxHash::digest()
{
    m_timer.start();

    if (m_finalized) {
        /* 已完成时返回上一次结果 */
        return mergeAccumulators();
    }
    m_finalized = true;

    quint64 result;
    if (m_stats.totalBytesProcessed >= STRIPE_LEN) {
        /* 长输入: 合并累加器 */
        result = mergeAccumulators();
    } else {
        /* 短输入: 简单路径 */
        result = m_seed + PRIME64_5;
    }

    result += static_cast<quint64>(m_stats.totalBytesProcessed);

    /* 处理缓冲区中剩余数据 */
    int bufLen = m_buffer.size();
    int pos = 0;

    /* 处理8字节段 */
    while (pos + 8 <= bufLen) {
        quint64 val = 0;
        for (int b = 0; b < 8; ++b) {
            val |= static_cast<quint64>(static_cast<quint8>(m_buffer[pos + b]))
                   << (8 * b);
        }
        result ^= mul128Fold64(val, PRIME64_2);
        result = (result << 27) | (result >> 37);
        result *= PRIME64_1;
        pos += 8;
    }

    /* 处理4字节段 */
    if (pos + 4 <= bufLen) {
        quint32 val = static_cast<quint32>(static_cast<quint8>(m_buffer[pos]))
                    | (static_cast<quint32>(static_cast<quint8>(m_buffer[pos + 1])) << 8)
                    | (static_cast<quint32>(static_cast<quint8>(m_buffer[pos + 2])) << 16)
                    | (static_cast<quint32>(static_cast<quint8>(m_buffer[pos + 3])) << 24);
        result ^= static_cast<quint64>(val) * PRIME64_1;
        result = (result << 23) | (result >> 41);
        result *= PRIME64_2;
        pos += 4;
    }

    /* 处理剩余单字节 */
    while (pos < bufLen) {
        result ^= static_cast<quint64>(static_cast<quint8>(m_buffer[pos]))
                   * PRIME64_5;
        result = (result << 11) | (result >> 53);
        result *= PRIME64_1;
        ++pos;
    }

    /* 最终混合(avalanche) */
    result ^= result >> 33;
    result *= PRIME64_2;
    result ^= result >> 29;
    result *= PRIME64_3;
    result ^= result >> 32;

    ++m_stats.totalHashes;
    qint64 elapsed = m_timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalHashes);

    emit hashComputed(static_cast<qint64>(m_stats.totalBytesProcessed));
    return result;
}

/** @brief 一次性计算哈希(静态便捷接口) @param data 输入数据 @param seed 种子值 @return 64位哈希值 */
quint64 XxHash::hash(const QByteArray& data, quint64 seed)
{
    /* 静态方法不更新实例统计 */
    int len = data.size();

    if (len >= STRIPE_LEN) {
        quint64 acc[4] = {
            seed + PRIME64_1 + PRIME64_2,
            seed + PRIME64_2,
            seed,
            seed - PRIME64_1
        };

        int stripes = len / STRIPE_LEN;
        int i = 0;
        for (; i < stripes * STRIPE_LEN; i += STRIPE_LEN) {
            for (int lane = 0; lane < 4; ++lane) {
                quint64 val = 0;
                for (int b = 0; b < 8; ++b) {
                    val |= static_cast<quint64>(
                        static_cast<quint8>(data[i + lane * 8 + b]))
                        << (8 * b);
                }
                acc[lane] += val * PRIME64_2;
                acc[lane] = (acc[lane] << 31) | (acc[lane] >> 33);
                acc[lane] *= PRIME64_1;
            }
        }

        quint64 result = (acc[0] << 1) | (acc[0] >> 63);
        result += (acc[1] << 7) | (acc[1] >> 57);
        result += (acc[2] << 12) | (acc[2] >> 52);
        result += (acc[3] << 18) | (acc[3] >> 46);

        result += static_cast<quint64>(len);

        /* 处理剩余数据 */
        int pos = stripes * STRIPE_LEN;
        while (pos + 8 <= len) {
            quint64 val = 0;
            for (int b = 0; b < 8; ++b) {
                val |= static_cast<quint64>(static_cast<quint8>(data[pos + b]))
                       << (8 * b);
            }
            result ^= mul128Fold64(val, PRIME64_2);
            result = (result << 27) | (result >> 37);
            result *= PRIME64_1;
            pos += 8;
        }

        if (pos + 4 <= len) {
            quint32 v32 = static_cast<quint32>(static_cast<quint8>(data[pos]))
                        | (static_cast<quint32>(static_cast<quint8>(data[pos + 1])) << 8)
                        | (static_cast<quint32>(static_cast<quint8>(data[pos + 2])) << 16)
                        | (static_cast<quint32>(static_cast<quint8>(data[pos + 3])) << 24);
            result ^= static_cast<quint64>(v32) * PRIME64_1;
            result = (result << 23) | (result >> 41);
            result *= PRIME64_2;
            pos += 4;
        }

        while (pos < len) {
            result ^= static_cast<quint64>(static_cast<quint8>(data[pos])) * PRIME64_5;
            result = (result << 11) | (result >> 53);
            result *= PRIME64_1;
            ++pos;
        }

        result ^= result >> 33;
        result *= PRIME64_2;
        result ^= result >> 29;
        result *= PRIME64_3;
        result ^= result >> 32;
        return result;
    }

    /* 短输入路径 */
    quint64 result = seed + PRIME64_5 + static_cast<quint64>(len);
    int pos = 0;
    while (pos + 8 <= len) {
        quint64 val = 0;
        for (int b = 0; b < 8; ++b) {
            val |= static_cast<quint64>(static_cast<quint8>(data[pos + b]))
                   << (8 * b);
        }
        result ^= mul128Fold64(val, PRIME64_2);
        result = (result << 27) | (result >> 37);
        result *= PRIME64_1;
        pos += 8;
    }
    while (pos < len) {
        result ^= static_cast<quint64>(static_cast<quint8>(data[pos])) * PRIME64_5;
        result = (result << 11) | (result >> 53);
        result *= PRIME64_1;
        ++pos;
    }
    result ^= result >> 33;
    result *= PRIME64_2;
    result ^= result >> 29;
    result *= PRIME64_3;
    result ^= result >> 32;
    return result;
}

/** @brief 合并累加器为最终哈希值 @return 64位结果 */
quint64 XxHash::mergeAccumulators() const
{
    quint64 result = (m_acc[0] << 1) | (m_acc[0] >> 63);
    result += (m_acc[1] << 7) | (m_acc[1] >> 57);
    result += (m_acc[2] << 12) | (m_acc[2] >> 52);
    result += (m_acc[3] << 18) | (m_acc[3] >> 46);
    return result;
}

/** @brief 混合步骤(64位乘法折叠) @param lhs 值A @param rhs 值B */
quint64 XxHash::mul128Fold64(quint64 lhs, quint64 rhs)
{
    quint64 hl = lhs >> 32, ll = lhs & 0xFFFFFFFFULL;
    quint64 hr = rhs >> 32, lr = rhs & 0xFFFFFFFFULL;
    quint64 cross = hl * lr + ll * hr;
    quint64 lo = ll * lr;
    quint64 hi = hl * hr + (cross >> 32);
    return hi ^ (lo + (cross << 32));
}

/** @brief 重置所有统计计数器 */
void XxHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
