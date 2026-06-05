/**
 * @file ChaChaCipher.cpp
 * @brief ChaCha20流密码实现
 */

#include "ChaChaCipher.h"
#include <QElapsedTimer>

ChaChaCipher::ChaChaCipher(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void ChaChaCipher::setKey(const QByteArray& key)
{
    m_key = key.left(32);
    if (m_key.size() < 32)
        m_key.resize(32, '\0');
}

void ChaChaCipher::setNonce(const QByteArray& nonce)
{
    m_nonce = nonce.left(12);
    if (m_nonce.size() < 12)
        m_nonce.resize(12, '\0');
}

quint32 ChaChaCipher::rotl32(quint32 x, int n)
{
    return (x << n) | (x >> (32 - n));
}

void ChaChaCipher::quarterRound(quint32& a, quint32& b, quint32& c, quint32& d)
{
    a += b; d ^= a; d = rotl32(d, 16);
    c += d; b ^= c; b = rotl32(b, 12);
    a += b; d ^= a; d = rotl32(d, 8);
    c += d; b ^= c; b = rotl32(b, 7);
}

void ChaChaCipher::innerBlock(quint32 state[16])
{
    for (int i = 0; i < 10; ++i) {
        /* 列轮 */
        quarterRound(state[0], state[4], state[8], state[12]);
        quarterRound(state[1], state[5], state[9], state[13]);
        quarterRound(state[2], state[6], state[10], state[14]);
        quarterRound(state[3], state[7], state[11], state[15]);
        /* 对角轮 */
        quarterRound(state[0], state[5], state[10], state[15]);
        quarterRound(state[1], state[6], state[11], state[12]);
        quarterRound(state[2], state[7], state[8], state[13]);
        quarterRound(state[3], state[4], state[9], state[14]);
    }
}

QByteArray ChaChaCipher::generateBlock(quint32 counter)
{
    /* 常数 "expand 32-byte k" */
    quint32 state[16] = {
        0x61707865, 0x3320646e, 0x79622d32, 0x6b206574,
        0, 0, 0, 0,
        0, 0, 0, 0,
        counter, 0, 0, 0
    };

    /* 填充密钥 */
    for (int i = 0; i < 8; ++i) {
        state[4 + i] = static_cast<quint32>(
            static_cast<quint8>(m_key[i * 4])) |
            (static_cast<quint32>(static_cast<quint8>(m_key[i * 4 + 1])) << 8) |
            (static_cast<quint32>(static_cast<quint8>(m_key[i * 4 + 2])) << 16) |
            (static_cast<quint32>(static_cast<quint8>(m_key[i * 4 + 3])) << 24);
    }

    /* 填充nonce */
    for (int i = 0; i < 3; ++i) {
        state[13 + i] = static_cast<quint32>(
            static_cast<quint8>(m_nonce[i * 4])) |
            (static_cast<quint32>(static_cast<quint8>(m_nonce[i * 4 + 1])) << 8) |
            (static_cast<quint32>(static_cast<quint8>(m_nonce[i * 4 + 2])) << 16) |
            (static_cast<quint32>(static_cast<quint8>(m_nonce[i * 4 + 3])) << 24);
    }

    quint32 working[16];
    for (int i = 0; i < 16; ++i) working[i] = state[i];

    innerBlock(working);

    QByteArray result(64, '\0');
    for (int i = 0; i < 16; ++i) {
        quint32 val = working[i] + state[i];
        result[i * 4] = static_cast<char>(val & 0xFF);
        result[i * 4 + 1] = static_cast<char>((val >> 8) & 0xFF);
        result[i * 4 + 2] = static_cast<char>((val >> 16) & 0xFF);
        result[i * 4 + 3] = static_cast<char>((val >> 24) & 0xFF);
    }

    return result;
}

QByteArray ChaChaCipher::process(const QByteArray& data, quint32 counter)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    result.reserve(data.size());

    int offset = 0;
    quint32 blockCounter = counter;

    while (offset < data.size()) {
        QByteArray block = generateBlock(blockCounter++);
        int chunkSize = qMin(64, data.size() - offset);

        for (int i = 0; i < chunkSize; ++i)
            result.append(data[offset + i] ^ block[i]);

        offset += chunkSize;
    }

    m_stats.totalEncrypted++;
    m_stats.totalBytes += data.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncrypted + m_stats.totalDecrypted;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit encryptionCompleted(data.size());
    return result;
}

QByteArray ChaChaCipher::encrypt(const QByteArray& plaintext, quint32 counter)
{
    m_stats.totalEncrypted--;
    return process(plaintext, counter);
}

QByteArray ChaChaCipher::decrypt(const QByteArray& ciphertext, quint32 counter)
{
    QElapsedTimer timer;
    timer.start();
    auto result = process(ciphertext, counter);
    m_stats.totalEncrypted--;
    m_stats.totalDecrypted++;
    return result;
}

ChaChaCipher::Stats ChaChaCipher::stats() const { return m_stats; }

void ChaChaCipher::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
