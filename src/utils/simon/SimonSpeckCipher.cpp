/**
 * @file SimonSpeckCipher.cpp
 * @brief Simon/Speck轻量级分组密码实现
 */

#include "utils/simon/SimonSpeckCipher.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
SimonSpeckCipher::SimonSpeckCipher(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置密钥 @param key 64位密钥 */
void SimonSpeckCipher::setKey(quint64 key)
{
    m_key = key;
    m_keyExpanded = false;
    /* 根据当前算法立即扩展轮密钥 */
    if (m_algo == Algorithm::Speck) {
        expandSpeckKey();
    } else {
        expandSimonKey();
    }
    m_keyExpanded = true;
}

/** @brief 选择算法 @param algo Simon或Speck */
void SimonSpeckCipher::setAlgorithm(Algorithm algo)
{
    m_algo = algo;
    m_keyExpanded = false;
    if (m_key != 0) {
        if (m_algo == Algorithm::Speck) {
            expandSpeckKey();
        } else {
            expandSimonKey();
        }
        m_keyExpanded = true;
    }
}

/**
 * @brief 加密数据
 * @param data 明文数据
 * @return 密文(PKCS7填充到8字节对齐)
 */
QByteArray SimonSpeckCipher::encrypt(const QByteArray& data)
{
    if (data.isEmpty()) return {};
    if (!m_keyExpanded) return {};

    m_timer.start();

    /* PKCS7填充到8字节对齐 */
    int padLen = 8 - (data.size() % 8);
    QByteArray padded(data);
    padded.append(QByteArray(padLen, static_cast<char>(padLen)));

    QByteArray result;
    result.resize(padded.size());

    for (int i = 0; i < padded.size(); i += 8) {
        /* 读取8字节块为两个32位半字(小端) */
        quint32 left = static_cast<quint8>(padded[i])
                     | (static_cast<quint32>(static_cast<quint8>(padded[i+1])) << 8)
                     | (static_cast<quint32>(static_cast<quint8>(padded[i+2])) << 16)
                     | (static_cast<quint32>(static_cast<quint8>(padded[i+3])) << 24);
        quint32 right = static_cast<quint8>(padded[i+4])
                      | (static_cast<quint32>(static_cast<quint8>(padded[i+5])) << 8)
                      | (static_cast<quint32>(static_cast<quint8>(padded[i+6])) << 16)
                      | (static_cast<quint32>(static_cast<quint8>(padded[i+7])) << 24);

        encryptBlock(left, right);

        /* 写回加密后的块 */
        result[i]   = static_cast<char>(left & 0xFF);
        result[i+1] = static_cast<char>((left >> 8) & 0xFF);
        result[i+2] = static_cast<char>((left >> 16) & 0xFF);
        result[i+3] = static_cast<char>((left >> 24) & 0xFF);
        result[i+4] = static_cast<char>(right & 0xFF);
        result[i+5] = static_cast<char>((right >> 8) & 0xFF);
        result[i+6] = static_cast<char>((right >> 16) & 0xFF);
        result[i+7] = static_cast<char>((right >> 24) & 0xFF);
    }

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalEncrypted;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalEncrypted + m_stats.totalDecrypted);

    emit encryptionCompleted(result.size());
    return result;
}

/**
 * @brief 解密数据
 * @param data 密文数据
 * @return 明文(去除PKCS7填充)
 */
QByteArray SimonSpeckCipher::decrypt(const QByteArray& data)
{
    if (data.size() < 8 || (data.size() % 8) != 0) return {};
    if (!m_keyExpanded) return {};

    m_timer.start();

    QByteArray result;
    result.resize(data.size());

    for (int i = 0; i < data.size(); i += 8) {
        quint32 left = static_cast<quint8>(data[i])
                     | (static_cast<quint32>(static_cast<quint8>(data[i+1])) << 8)
                     | (static_cast<quint32>(static_cast<quint8>(data[i+2])) << 16)
                     | (static_cast<quint32>(static_cast<quint8>(data[i+3])) << 24);
        quint32 right = static_cast<quint8>(data[i+4])
                      | (static_cast<quint32>(static_cast<quint8>(data[i+5])) << 8)
                      | (static_cast<quint32>(static_cast<quint8>(data[i+6])) << 16)
                      | (static_cast<quint32>(static_cast<quint8>(data[i+7])) << 24);

        decryptBlock(left, right);

        result[i]   = static_cast<char>(left & 0xFF);
        result[i+1] = static_cast<char>((left >> 8) & 0xFF);
        result[i+2] = static_cast<char>((left >> 16) & 0xFF);
        result[i+3] = static_cast<char>((left >> 24) & 0xFF);
        result[i+4] = static_cast<char>(right & 0xFF);
        result[i+5] = static_cast<char>((right >> 8) & 0xFF);
        result[i+6] = static_cast<char>((right >> 16) & 0xFF);
        result[i+7] = static_cast<char>((right >> 24) & 0xFF);
    }

    /* 去除PKCS7填充 */
    int padVal = static_cast<quint8>(result[result.size() - 1]);
    if (padVal >= 1 && padVal <= 8) {
        result.chop(padVal);
    }

    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalDecrypted;
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalEncrypted + m_stats.totalDecrypted);

    emit encryptionCompleted(result.size());
    return result;
}

/** @brief 重置统计 */
void SimonSpeckCipher::resetStatistics()
{
    m_stats = Stats{};
    m_totalTimeMs = 0.0;
}

/**
 * @brief Speck单轮加密
 * Speck64/96: x = ROR(x,8), y = ROL(y,3), y ^= x + k, x ^= y
 */
void SimonSpeckCipher::speckRound(quint32& x, quint32& y, quint32 k) const
{
    x = ((x >> 8) | (x << 24)) & MASK32;       /* ROR 8 */
    y = ((y << 3) | (y >> 29)) & MASK32;       /* ROL 3 */
    y ^= (x + k) & MASK32;
    x ^= y;
}

/**
 * @brief Speck单轮解密(逆)
 */
void SimonSpeckCipher::speckRoundInv(quint32& x, quint32& y, quint32 k) const
{
    x ^= y;
    y ^= (x + k) & MASK32;
    y = ((y >> 3) | (y << 29)) & MASK32;       /* ROR 3 */
    x = ((x << 8) | (x >> 24)) & MASK32;       /* ROL 8 */
}

/**
 * @brief Simon单轮加密
 * Simon64/128: tmp = x, x = ROL(x,1) & ROL(x,8) ^ ROL(x,2) ^ y ^ k, y = tmp
 */
void SimonSpeckCipher::simonRound(quint32& x, quint32& y, quint32 k) const
{
    quint32 tmp = x;
    quint32 x1 = ((x << 1) | (x >> 31)) & MASK32;   /* ROL 1 */
    quint32 x8 = ((x << 8) | (x >> 24)) & MASK32;   /* ROL 8 */
    quint32 x2 = ((x << 2) | (x >> 30)) & MASK32;   /* ROL 2 */
    x = (x1 & x8) ^ x2 ^ y ^ k;
    y = tmp;
}

/**
 * @brief Simon单轮解密(逆)
 */
void SimonSpeckCipher::simonRoundInv(quint32& x, quint32& y, quint32 k) const
{
    quint32 tmp = y;
    quint32 y1 = ((y << 1) | (y >> 31)) & MASK32;
    quint32 y8 = ((y << 8) | (y >> 24)) & MASK32;
    quint32 y2 = ((y << 2) | (y >> 30)) & MASK32;
    y = (y1 & y8) ^ y2 ^ x ^ k;
    x = tmp;
}

/** @brief 生成Speck轮密钥 — Speck64/96密钥扩展 */
void SimonSpeckCipher::expandSpeckKey()
{
    /* 密钥k[0] || k[1] || k[2] -> 96位 */
    quint32 k0 = static_cast<quint32>(m_key & MASK32);
    quint32 k1 = static_cast<quint32>((m_key >> 32) & MASK32);
    quint32 k2 = 0; /* 第三字(简化为低位异或) */
    k2 = k0 ^ k1;

    m_subkeys[0] = k0;
    for (int i = 0; i < ROUNDS - 1; ++i) {
        /* k_{i+1} = ROR(k2, 8) ^ (i+1) ^ ROL(k1, 3) */
        quint32 rotated = ((k2 >> 8) | (k2 << 24)) & MASK32;
        k1 = ((k1 << 3) | (k1 >> 29)) & MASK32;
        k2 = rotated ^ static_cast<quint32>(i + 1) ^ k1;
        m_subkeys[i + 1] = k2;
        /* 循环移位密钥字 */
        quint32 tmp = k0;
        k0 = k1;
        k1 = k2;
        k2 = tmp;
    }
}

/** @brief 生成Simon轮密钥 — Simon64/128密钥扩展 */
void SimonSpeckCipher::expandSimonKey()
{
    quint64 fullKey = m_key;
    quint32 k0 = static_cast<quint32>(fullKey & MASK32);
    quint32 k1 = static_cast<quint32>((fullKey >> 32) & MASK32);

    m_subkeys[0] = k0;

    for (int i = 0; i < ROUNDS - 1; ++i) {
        /* Simon密钥扩展: k_{i+2} = c ^ z_j ^ ROR(k_{i+1},3) ^ k_i ^ ROR(k_i,4) */
        quint32 tmp = k1;
        quint32 ror3 = ((k1 >> 3) | (k1 << 29)) & MASK32;
        quint32 ror4 = ((k0 >> 4) | (k0 << 28)) & MASK32;
        k1 = ror3 ^ k0 ^ ror4 ^ static_cast<quint32>(i);
        k0 = tmp;
        m_subkeys[i + 1] = k1;
    }
}

/** @brief 加密单个8字节块 */
void SimonSpeckCipher::encryptBlock(quint32& left, quint32& right) const
{
    if (m_algo == Algorithm::Speck) {
        for (int i = 0; i < ROUNDS; ++i) {
            speckRound(left, right, m_subkeys[i]);
        }
    } else {
        for (int i = 0; i < ROUNDS; ++i) {
            simonRound(left, right, m_subkeys[i]);
        }
    }
}

/** @brief 解密单个8字节块 */
void SimonSpeckCipher::decryptBlock(quint32& left, quint32& right) const
{
    if (m_algo == Algorithm::Speck) {
        for (int i = ROUNDS - 1; i >= 0; --i) {
            speckRoundInv(left, right, m_subkeys[i]);
        }
    } else {
        for (int i = ROUNDS - 1; i >= 0; --i) {
            simonRoundInv(left, right, m_subkeys[i]);
        }
    }
}
