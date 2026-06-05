/**
 * @file SpookyHash.cpp
 * @brief SpookyHash实现 — Bob Jenkins高速非加密哈希
 */

#include "utils/spooky/SpookyHash.h"

#include <QElapsedTimer>
#include <QtEndian>
#include <cstring>

/* ═══════════════════════════════════════════════════════════════
 *  SpookyHash 常量
 * ═══════════════════════════════════════════════════════════════ */
/** @brief SpookyHash初始状态值 */
static const quint64 SC_CONST = 0xdeadbeefdeadbeefULL;

/* ═══════════════════════════════════════════════════════════════
 *  构造 / 配置
 * ═══════════════════════════════════════════════════════════════ */

/** @brief 构造函数 @param parent 父对象 */
SpookyHash::SpookyHash(QObject* parent)
    : QObject(parent)
    , m_seed1(SC_CONST)
    , m_seed2(SC_CONST)
    , m_timeSumMs(0.0)
{
}

/** @brief 设置哈希种子 @param seed1 第一种子 @param seed2 第二种子 */
void SpookyHash::setSeed(quint64 seed1, quint64 seed2)
{
    m_seed1 = seed1;
    m_seed2 = seed2;
}

/* ═══════════════════════════════════════════════════════════════
 *  内部: 12状态混合
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 混合12个状态变量 (一轮)
 *
 * 每次从输入读取12个quint64，与12个状态变量进行混合。
 * 混合采用旋转+乘法+异或的组合，保证雪崩效应。
 *
 * @param data 12个quint64输入值
 * @param s    12个状态变量(输入/输出)
 */
void SpookyHash::mix(const quint64* data, quint64* s)
{
    for (int i = 0; i < 12; ++i) {
        s[i] += data[i];
    }
    /* 第一轮混合: 交叉异或 + 旋转 + 乘法 */
    s[ 0] ^= s[11]; s[ 0] = (s[0] << 11) | (s[0] >> 53);
    s[11] += s[ 0];
    s[ 1] ^= s[ 0]; s[ 1] = (s[1] << 32) | (s[1] >> 32);
    s[ 0] += s[ 1];

    s[ 2] ^= s[ 1]; s[ 2] = (s[2] << 43) | (s[2] >> 21);
    s[ 1] += s[ 2];
    s[ 3] ^= s[ 2]; s[ 3] = (s[3] << 31) | (s[3] >> 33);
    s[ 2] += s[ 3];

    s[ 4] ^= s[ 3]; s[ 4] = (s[4] << 17) | (s[4] >> 47);
    s[ 3] += s[ 4];
    s[ 5] ^= s[ 4]; s[ 5] = (s[5] << 28) | (s[5] >> 36);
    s[ 4] += s[ 5];

    s[ 6] ^= s[ 5]; s[ 6] = (s[6] << 39) | (s[6] >> 25);
    s[ 5] += s[ 6];
    s[ 7] ^= s[ 6]; s[ 7] = (s[7] << 57) | (s[7] >> 7);
    s[ 6] += s[ 7];

    s[ 8] ^= s[ 7]; s[ 8] = (s[8] << 55) | (s[8] >> 9);
    s[ 7] += s[ 8];
    s[ 9] ^= s[ 8]; s[ 9] = (s[9] << 54) | (s[9] >> 10);
    s[ 8] += s[ 9];

    s[10] ^= s[ 9]; s[10] = (s[10] << 22) | (s[10] >> 42);
    s[ 9] += s[10];
    s[11] ^= s[10]; s[11] = (s[11] << 46) | (s[11] >> 18);
    s[10] += s[11];
}

/**
 * @brief 最终混合步骤
 *
 * 对12个状态变量做最终雪崩混合，将状态收敛到输出。
 *
 * @param s 12个状态变量(输入/输出)
 */
void SpookyHash::end(quint64* s)
{
    /* 多轮交叉混合，确保充分扩散 */
    for (int round = 0; round < 3; ++round) {
        s[ 0] += s[ 1]; s[ 1] = (s[1] << 44) | (s[1] >> 20); s[ 1] ^= s[ 0];
        s[ 2] += s[ 3]; s[ 3] = (s[3] << 15) | (s[3] >> 49); s[ 3] ^= s[ 2];
        s[ 4] += s[ 5]; s[ 5] = (s[5] << 34) | (s[5] >> 30); s[ 5] ^= s[ 4];
        s[ 6] += s[ 7]; s[ 7] = (s[7] << 21) | (s[7] >> 43); s[ 7] ^= s[ 6];
        s[ 8] += s[ 9]; s[ 9] = (s[9] << 13) | (s[9] >> 51); s[ 9] ^= s[ 8];
        s[10] += s[11]; s[11] = (s[11]<< 38) | (s[11]>> 26); s[11] ^= s[10];
    }
}

/* ═══════════════════════════════════════════════════════════════
 *  内部: 短消息处理
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 短消息哈希 (<=192字节)
 *
 * 使用简化的变量混合，直接处理尾部字节。
 *
 * @param message 数据指针
 * @param length  数据长度
 * @param h1      种子1(输入/输出)
 * @param h2      种子2(输入/输出)
 */
void SpookyHash::shortHash(const void* message, int length,
                           quint64& h1, quint64& h2)
{
    const quint8* ptr = static_cast<const quint8*>(message);
    quint64 a = h1;
    quint64 b = h2;

    /* 处理8字节块 */
    while (length >= 8) {
        quint64 v = qFromLittleEndian<quint64>(ptr);
        a ^= v;
        a = (a << 50) | (a >> 14);
        a += b;
        b = (b << 52) | (b >> 12);
        b ^= a;
        ptr += 8;
        length -= 8;
    }

    /* 处理剩余字节 */
    quint64 v = 0;
    switch (length) {
    case 7: v ^= static_cast<quint64>(ptr[6]) << 48; Q_FALLTHROUGH();
    case 6: v ^= static_cast<quint64>(ptr[5]) << 40; Q_FALLTHROUGH();
    case 5: v ^= static_cast<quint64>(ptr[4]) << 32; Q_FALLTHROUGH();
    case 4: v ^= static_cast<quint64>(ptr[3]) << 24; Q_FALLTHROUGH();
    case 3: v ^= static_cast<quint64>(ptr[2]) << 16; Q_FALLTHROUGH();
    case 2: v ^= static_cast<quint64>(ptr[1]) << 8;  Q_FALLTHROUGH();
    case 1: v ^= static_cast<quint64>(ptr[0]);
        a ^= v;
        a = (a << 36) | (a >> 28);
        a += b;
        b = (b << 19) | (b >> 45);
        b ^= a;
        break;
    default: break;
    }

    /* 最终混合 */
    a ^= SC_CONST;
    b ^= SC_CONST;
    a = (a << 24) | (a >> 40);
    a += b; b = (b << 36) | (b >> 28); b ^= a;
    a = (a << 13) | (a >> 51); a += b;
    b = (b << 7)  | (b >> 57); b ^= a;

    h1 = a;
    h2 = b;
}

/* ═══════════════════════════════════════════════════════════════
 *  公开接口
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief 计算64位SpookyHash
 * @param data 输入数据
 * @return 64位哈希值
 */
quint64 SpookyHash::hash64(const QByteArray& data)
{
    auto result = hash128(data);
    return result.first ^ result.second;
}

/**
 * @brief 计算128位SpookyHash
 *
 * 短消息(<=192字节)使用shortHash;
 * 长消息分块处理，每块96字节(12个quint64)。
 *
 * @param data 输入数据
 * @return 128位哈希值 (QPair<高位, 低位>)
 */
QPair<quint64, quint64> SpookyHash::hash128(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    const int len = data.size();
    const quint8* ptr = reinterpret_cast<const quint8*>(data.constData());
    quint64 h1 = m_seed1;
    quint64 h2 = m_seed2;

    if (len <= 192) {
        /* 短消息路径 */
        shortHash(ptr, len, h1, h2);
    } else {
        /* 长消息: 初始化12个状态 */
        quint64 s[12];
        s[ 0] = h1; s[ 1] = h2; s[ 2] = SC_CONST;
        s[ 3] = SC_CONST; s[ 4] = SC_CONST; s[ 5] = SC_CONST;
        s[ 6] = SC_CONST; s[ 7] = SC_CONST; s[ 8] = SC_CONST;
        s[ 9] = SC_CONST; s[10] = SC_CONST; s[11] = SC_CONST;

        /* 分块处理，每块96字节 */
        int remaining = len;
        int offset = 0;
        while (remaining >= 96) {
            const quint64* block = reinterpret_cast<const quint64*>(ptr + offset);
            mix(block, s);
            offset += 96;
            remaining -= 96;
        }

        /* 尾部: 填充到96字节 */
        quint64 tailBlock[12] = {};
        const quint8* tailPtr = ptr + offset;
        int tailLen = remaining;
        for (int i = 0; i < tailLen && i < 96; ++i) {
            tailBlock[i / 8] |= static_cast<quint64>(tailPtr[i])
                              << ((i % 8) * 8);
        }
        tailBlock[11] = static_cast<quint64>(len);
        mix(tailBlock, s);
        end(s);

        h1 = s[0] ^ s[1] ^ s[2]  ^ s[3];
        h2 = s[4] ^ s[5] ^ s[6]  ^ s[7];
    }

    /* 更新统计 */
    qint64 elapsed = timer.nsecsElapsed();
    double ms = static_cast<double>(elapsed) / 1e6;
    ++m_stats.totalHashes;
    m_stats.totalBytesProcessed += static_cast<quint64>(len);
    m_timeSumMs += ms;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalHashes;

    emit hashComputed(static_cast<qint64>(len));
    return {h1, h2};
}

/** @brief 重置所有统计计数器 */
void SpookyHash::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
