/**
 * @file AesCbc.cpp
 * @brief AES-CBC加密/解密实现 — 纯C++ AES实现
 */

#include "utils/aes2/AesCbc.h"

#include <QElapsedTimer>
#include <QtCore/qrandom.h>
#include <QtGlobal>

#include <cstring>

/* AES S-Box */
const quint8 AesCbc::SBOX[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

/* AES 逆S-Box */
const quint8 AesCbc::INV_SBOX[256] = {
    0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
    0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
    0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
    0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
    0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
    0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
    0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
    0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
    0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
    0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
    0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
    0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
    0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
    0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
    0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
    0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
};

/* 轮常量 */
const quint8 AesCbc::RCON[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

/** @brief 构造函数 @param keySize 密钥长度 @param parent 父对象 */
AesCbc::AesCbc(KeySize keySize, QObject* parent)
    : QObject(parent)
    , m_keySize(keySize)
{
    /* 根据密钥长度确定轮数: AES128=10, AES192=12, AES256=14 */
    switch (keySize) {
    case AES128: m_numRounds = 10; break;
    case AES192: m_numRounds = 12; break;
    case AES256: m_numRounds = 14; break;
    default:     m_numRounds = 14; m_keySize = AES256; break;
    }
}

/** @brief 设置密钥和IV @param key 密钥 @param iv IV @return 是否成功 */
bool AesCbc::setKeyAndIv(const QByteArray& key, const QByteArray& iv)
{
    if (key.size() != m_keySize || iv.size() != 16) return false;
    m_key = key;
    m_iv = iv;
    m_roundKeys = keyExpansion(key);
    m_keySet = true;
    return true;
}

/** @brief 密钥扩展 @param key 原始密钥 @return 轮密钥 */
std::vector<quint32> AesCbc::keyExpansion(const QByteArray& key) const
{
    int nk = key.size() / 4;  /* 4, 6, or 8 words */
    int nr = m_numRounds;
    int totalWords = 4 * (nr + 1);

    std::vector<quint32> w(totalWords);

    /* 前nk个word直接从密钥复制 */
    for (int i = 0; i < nk; ++i) {
        w[i] = (static_cast<quint32>(static_cast<quint8>(key[4*i])) << 24)
             | (static_cast<quint32>(static_cast<quint8>(key[4*i+1])) << 16)
             | (static_cast<quint32>(static_cast<quint8>(key[4*i+2])) << 8)
             | static_cast<quint32>(static_cast<quint8>(key[4*i+3]));
    }

    for (int i = nk; i < totalWords; ++i) {
        quint32 temp = w[i - 1];
        if (i % nk == 0) {
            /* RotWord + SubWord + Rcon */
            temp = (temp << 8) | (temp >> 24);
            temp = (static_cast<quint32>(SBOX[(temp >> 24) & 0xFF]) << 24)
                 | (static_cast<quint32>(SBOX[(temp >> 16) & 0xFF]) << 16)
                 | (static_cast<quint32>(SBOX[(temp >> 8) & 0xFF]) << 8)
                 | static_cast<quint32>(SBOX[temp & 0xFF]);
            temp ^= static_cast<quint32>(RCON[i / nk]) << 24;
        } else if (nk > 6 && (i % nk == 4)) {
            temp = (static_cast<quint32>(SBOX[(temp >> 24) & 0xFF]) << 24)
                 | (static_cast<quint32>(SBOX[(temp >> 16) & 0xFF]) << 16)
                 | (static_cast<quint32>(SBOX[(temp >> 8) & 0xFF]) << 8)
                 | static_cast<quint32>(SBOX[temp & 0xFF]);
        }
        w[i] = w[i - nk] ^ temp;
    }
    return w;
}

/** @brief SubBytes @param state 状态 */
void AesCbc::subBytes(quint8* state) const
{
    for (int i = 0; i < 16; ++i) state[i] = SBOX[state[i]];
}

/** @brief 逆SubBytes @param state 状态 */
void AesCbc::invSubBytes(quint8* state) const
{
    for (int i = 0; i < 16; ++i) state[i] = INV_SBOX[state[i]];
}

/** @brief ShiftRows @param state 状态 */
void AesCbc::shiftRows(quint8* state) const
{
    quint8 t;
    /* Row 1: 左移1 */
    t = state[1]; state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = t;
    /* Row 2: 左移2 */
    t = state[2]; state[2] = state[10]; state[10] = t;
    t = state[6]; state[6] = state[14]; state[14] = t;
    /* Row 3: 左移3 */
    t = state[15]; state[15] = state[11]; state[11] = state[7]; state[7] = state[3]; state[3] = t;
}

/** @brief 逆ShiftRows @param state 状态 */
void AesCbc::invShiftRows(quint8* state) const
{
    quint8 t;
    t = state[13]; state[13] = state[9]; state[9] = state[5]; state[5] = state[1]; state[1] = t;
    t = state[2]; state[2] = state[10]; state[10] = t;
    t = state[6]; state[6] = state[14]; state[14] = t;
    t = state[3]; state[3] = state[7]; state[7] = state[11]; state[11] = state[15]; state[15] = t;
}

/** @brief xtime (GF(2^8)上乘2) */
static inline quint8 xtime(quint8 x) { return static_cast<quint8>((x << 1) ^ ((x & 0x80) ? 0x1b : 0x00)); }

/** @brief MixColumns @param state 状态 */
void AesCbc::mixColumns(quint8* state) const
{
    for (int c = 0; c < 4; ++c) {
        int i = c * 4;
        quint8 a0 = state[i], a1 = state[i+1], a2 = state[i+2], a3 = state[i+3];
        state[i]   = xtime(a0) ^ xtime(a1) ^ a1 ^ a2 ^ a3;
        state[i+1] = a0 ^ xtime(a1) ^ xtime(a2) ^ a2 ^ a3;
        state[i+2] = a0 ^ a1 ^ xtime(a2) ^ xtime(a3) ^ a3;
        state[i+3] = xtime(a0) ^ a0 ^ a1 ^ a2 ^ xtime(a3);
    }
}

/** @brief GF乘法辅助 */
static inline quint8 gmul(quint8 a, quint8 b)
{
    quint8 p = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) p ^= a;
        quint8 hi = a & 0x80;
        a <<= 1;
        if (hi) a ^= 0x1b;
        b >>= 1;
    }
    return p;
}

/** @brief 逆MixColumns @param state 状态 */
void AesCbc::invMixColumns(quint8* state) const
{
    for (int c = 0; c < 4; ++c) {
        int i = c * 4;
        quint8 a0 = state[i], a1 = state[i+1], a2 = state[i+2], a3 = state[i+3];
        state[i]   = gmul(a0,0x0e) ^ gmul(a1,0x0b) ^ gmul(a2,0x0d) ^ gmul(a3,0x09);
        state[i+1] = gmul(a0,0x09) ^ gmul(a1,0x0e) ^ gmul(a2,0x0b) ^ gmul(a3,0x0d);
        state[i+2] = gmul(a0,0x0d) ^ gmul(a1,0x09) ^ gmul(a2,0x0e) ^ gmul(a3,0x0b);
        state[i+3] = gmul(a0,0x0b) ^ gmul(a1,0x0d) ^ gmul(a2,0x09) ^ gmul(a3,0x0e);
    }
}

/** @brief AddRoundKey @param state 状态 @param roundKeys 轮密钥 @param round 轮次 */
void AesCbc::addRoundKey(quint8* state, const std::vector<quint32>& roundKeys, int round) const
{
    for (int c = 0; c < 4; ++c) {
        quint32 w = roundKeys[round * 4 + c];
        state[c*4]     ^= (w >> 24) & 0xFF;
        state[c*4 + 1] ^= (w >> 16) & 0xFF;
        state[c*4 + 2] ^= (w >> 8) & 0xFF;
        state[c*4 + 3] ^= w & 0xFF;
    }
}

/** @brief 单块加密 @param state 状态 @param roundKeys 轮密钥 */
void AesCbc::encryptBlock(quint8* state, const std::vector<quint32>& roundKeys) const
{
    addRoundKey(state, roundKeys, 0);
    for (int r = 1; r < m_numRounds; ++r) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, roundKeys, r);
    }
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, roundKeys, m_numRounds);
}

/** @brief 单块解密 @param state 状态 @param roundKeys 轮密钥 */
void AesCbc::decryptBlock(quint8* state, const std::vector<quint32>& roundKeys) const
{
    addRoundKey(state, roundKeys, m_numRounds);
    for (int r = m_numRounds - 1; r >= 1; --r) {
        invShiftRows(state);
        invSubBytes(state);
        addRoundKey(state, roundKeys, r);
        invMixColumns(state);
    }
    invShiftRows(state);
    invSubBytes(state);
    addRoundKey(state, roundKeys, 0);
}

/** @brief PKCS7填充 @param data 原始数据 @return 填充后数据 */
QByteArray AesCbc::pkcs7Pad(const QByteArray& data) const
{
    int padLen = 16 - (data.size() % 16);
    QByteArray padded = data;
    padded.append(padLen, static_cast<char>(padLen));
    return padded;
}

/** @brief PKCS7去填充 @param data 填充数据 @return 原始数据 */
QByteArray AesCbc::pkcs7Unpad(const QByteArray& data) const
{
    if (data.isEmpty() || data.size() % 16 != 0) return {};
    int padLen = static_cast<quint8>(data.back());
    if (padLen < 1 || padLen > 16) return {};
    for (int i = data.size() - padLen; i < data.size(); ++i) {
        if (static_cast<quint8>(data[i]) != padLen) return {};
    }
    return data.left(data.size() - padLen);
}

/** @brief CBC加密 @param plaintext 明文 @return 密文 */
QByteArray AesCbc::encrypt(const QByteArray& plaintext)
{
    if (!m_keySet) return {};

    QElapsedTimer timer;
    timer.start();

    QByteArray padded = pkcs7Pad(plaintext);
    QByteArray cipher;
    cipher.resize(padded.size());

    quint8 prev[16];
    std::memcpy(prev, m_iv.constData(), 16);

    for (int off = 0; off < padded.size(); off += 16) {
        quint8 block[16];
        /* CBC: plaintext XOR previous ciphertext */
        for (int i = 0; i < 16; ++i) {
            block[i] = static_cast<quint8>(padded[off + i]) ^ prev[i];
        }
        encryptBlock(block, m_roundKeys);
        std::memcpy(prev, block, 16);
        std::memcpy(cipher.data() + off, block, 16);
    }

    m_stats.totalBytesEncrypted += static_cast<quint64>(plaintext.size());
    ++m_stats.totalEncryptions;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit encryptionCompleted(cipher.size());
    return cipher;
}

/** @brief CBC解密 @param ciphertext 密文 @return 明文 */
QByteArray AesCbc::decrypt(const QByteArray& ciphertext)
{
    if (!m_keySet || ciphertext.isEmpty() || ciphertext.size() % 16 != 0) return {};

    QElapsedTimer timer;
    timer.start();

    QByteArray plain;
    plain.resize(ciphertext.size());

    quint8 prev[16];
    std::memcpy(prev, m_iv.constData(), 16);

    for (int off = 0; off < ciphertext.size(); off += 16) {
        quint8 block[16];
        std::memcpy(block, ciphertext.constData() + off, 16);

        quint8 saved[16];
        std::memcpy(saved, block, 16);

        decryptBlock(block, m_roundKeys);

        /* CBC: plaintext = decrypt(cipher) XOR previous ciphertext */
        for (int i = 0; i < 16; ++i) {
            plain[off + i] = static_cast<char>(block[i] ^ prev[i]);
        }
        std::memcpy(prev, saved, 16);
    }

    QByteArray result = pkcs7Unpad(plain);

    m_stats.totalBytesDecrypted += static_cast<quint64>(ciphertext.size());
    ++m_stats.totalDecryptions;
    m_timeSumMs += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSumMs
        / static_cast<double>(m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit decryptionCompleted(result.size());
    return result;
}

/** @brief 生成随机密钥 @return 随机密钥 */
QByteArray AesCbc::generateKey() const
{
    QByteArray key(m_keySize, '\0');
    for (int i = 0; i < m_keySize; ++i) {
        key[i] = static_cast<char>(QRandomGenerator::global()->generate() & 0xFF);
    }
    return key;
}

/** @brief 生成随机IV @return 16字节随机IV */
QByteArray AesCbc::generateIv() const
{
    QByteArray iv(16, '\0');
    for (int i = 0; i < 16; ++i) {
        iv[i] = static_cast<char>(QRandomGenerator::global()->generate() & 0xFF);
    }
    return iv;
}

/** @brief 重置统计 */
void AesCbc::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
