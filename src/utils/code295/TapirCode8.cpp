/**
 * @file TapirCode8.cpp
 * @brief TapirCode8 实现
 *
 * 实现Tapir编码：变长符号编码与位置相关XOR扩散实现轻量级流密码。
 */

#include "utils/code295/TapirCode8.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

TapirCode8::TapirCode8(QObject *parent)
    : QObject(parent) {}

TapirCode8::~TapirCode8() = default;

/* ---- Configuration ---- */

void TapirCode8::setKey(quint64 key) { m_key = key; }

/* ---- Keystream byte generation (position-dependent) ---- */

quint8 TapirCode8::keystreamByte(int position) const
{
    // Combine key with position using multiplicative hash + XOR rotation
    quint64 h = m_key;
    h ^= static_cast<quint64>(position) * 0x9E3779B97F4A7C15ULL;
    h = (h ^ (h >> 30)) * 0xBF58476D1CE4E5B9ULL;
    h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
    h ^= (h >> 31);
    // Mix with position-dependent rotation
    h ^= static_cast<quint64>(position) << (position & 7);
    return static_cast<quint8>(h ^ (h >> 8) ^ (h >> 16) ^ (h >> 24));
}

/* ---- Variable-length symbol encoding ---- */

QVector<quint8> TapirCode8::varLengthEncode(const QVector<quint8>& data) const
{
    // Pack nibbles: if two consecutive bytes share high nibble pattern,
    // encode as (flag|low_nibble_pair) otherwise as raw byte
    QVector<quint8> encoded;
    encoded.reserve(data.size());
    int i = 0;
    while (i < data.size()) {
        // Check if current and next byte can be nibble-packed
        if (i + 1 < data.size()) {
            quint8 hi1 = (data[i] >> 4) & 0x0F;
            quint8 hi2 = (data[i + 1] >> 4) & 0x0F;
            // If both high nibbles are zero (small values), pack as single byte
            if (hi1 == 0 && hi2 == 0) {
                quint8 packed = 0x80 | ((data[i] & 0x0F) << 4) | (data[i + 1] & 0x0F);
                encoded.append(packed);
                i += 2;
                continue;
            }
        }
        // Raw byte with high bit clear
        quint8 raw = data[i] & 0x7F;
        // If high bit was set, encode as escape + raw
        if (data[i] & 0x80) {
            encoded.append(0x40 | (data[i] >> 4));
            encoded.append(data[i] & 0x0F);
        } else {
            encoded.append(raw);
        }
        i++;
    }
    return encoded;
}

/* ---- Variable-length symbol decoding ---- */

QVector<quint8> TapirCode8::varLengthDecode(const QVector<quint8>& encoded,
                                             int originalSize) const
{
    QVector<quint8> decoded;
    decoded.reserve(originalSize);
    int i = 0;
    while (i < encoded.size() && decoded.size() < originalSize) {
        quint8 b = encoded[i];
        if (b & 0x80) {
            // Packed nibble pair
            decoded.append((b >> 4) & 0x0F);
            if (decoded.size() < originalSize)
                decoded.append(b & 0x0F);
            i++;
        } else if (b & 0x40) {
            // Escaped high-nibble
            quint8 hi = (b & 0x3F) << 4;
            if (i + 1 < encoded.size()) {
                i++;
                decoded.append(hi | (encoded[i] & 0x0F));
            }
            i++;
        } else {
            decoded.append(b);
            i++;
        }
    }
    return decoded;
}

/* ---- XOR diffusion layer ---- */

void TapirCode8::xorDiffusion(QVector<quint8>& block, int basePos) const
{
    int sz = block.size();
    if (sz == 0) return;

    // Forward pass: position-dependent XOR with keystream
    for (int i = 0; i < sz; ++i) {
        quint8 ks = keystreamByte(basePos + i);
        block[i] ^= ks;
    }

    // Diffusion: each byte XORed with previous cipher byte (CBC-like chaining)
    for (int i = 1; i < sz; ++i) {
        block[i] ^= block[i - 1];
    }

    // Second round with reversed keystream
    for (int i = 0; i < sz; ++i) {
        quint8 ks = keystreamByte(basePos + sz - 1 - i);
        block[i] ^= ks;
    }
}

/* ---- Reverse XOR diffusion ---- */

void TapirCode8::xorDiffusionInverse(QVector<quint8>& block, int basePos) const
{
    int sz = block.size();
    if (sz == 0) return;

    // Reverse second keystream round
    for (int i = 0; i < sz; ++i) {
        quint8 ks = keystreamByte(basePos + sz - 1 - i);
        block[i] ^= ks;
    }

    // Reverse diffusion chaining (backward)
    for (int i = sz - 1; i > 0; --i) {
        block[i] ^= block[i - 1];
    }

    // Reverse first keystream round
    for (int i = 0; i < sz; ++i) {
        quint8 ks = keystreamByte(basePos + i);
        block[i] ^= ks;
    }
}

/* ---- Encode ---- */

TapirCode8::EncodeResult TapirCode8::encode(const QVector<quint8>& data)
{
    QElapsedTimer timer;
    timer.start();

    EncodeResult result;
    result.originalSize = data.size();

    // Step 1: Variable-length encode for compression
    auto varEncoded = varLengthEncode(data);

    // Step 2: Apply XOR diffusion cipher in 16-byte blocks
    int blockSize = 16;
    QVector<quint8> cipher = varEncoded;

    for (int i = 0; i < cipher.size(); i += blockSize) {
        int end = qMin(i + blockSize, cipher.size());
        QVector<quint8> block(cipher.mid(i, end - i));
        xorDiffusion(block, i);
        for (int j = 0; j < block.size(); ++j)
            cipher[i + j] = block[j];
    }

    // Prepend header: 2 bytes original size (big endian)
    result.encoded.resize(2 + cipher.size());
    result.encoded[0] = static_cast<quint8>((data.size() >> 8) & 0xFF);
    result.encoded[1] = static_cast<quint8>(data.size() & 0xFF);
    for (int i = 0; i < cipher.size(); ++i)
        result.encoded[2 + i] = cipher[i];

    result.encodedSize = result.encoded.size();
    result.compressionRatio = (data.size() > 0)
                                  ? static_cast<double>(result.encodedSize) / data.size()
                                  : 0.0;

    double elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_stats.lastBlockSize = data.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeDone(data.size(), result.compressionRatio, elapsed);
    return result;
}

/* ---- Decode ---- */

QVector<quint8> TapirCode8::decode(const QVector<quint8>& encoded, int originalSize)
{
    QElapsedTimer timer;
    timer.start();

    if (encoded.size() < 2) return {};

    // Extract original size from header
    int origSize = (static_cast<int>(encoded[0]) << 8) | encoded[1];

    // Extract cipher text
    QVector<quint8> cipher(encoded.size() - 2);
    for (int i = 0; i < cipher.size(); ++i)
        cipher[i] = encoded[2 + i];

    // Step 1: Reverse XOR diffusion in 16-byte blocks
    int blockSize = 16;
    for (int i = 0; i < cipher.size(); i += blockSize) {
        int end = qMin(i + blockSize, cipher.size());
        QVector<quint8> block(cipher.mid(i, end - i));
        xorDiffusionInverse(block, i);
        for (int j = 0; j < block.size(); ++j)
            cipher[i + j] = block[j];
    }

    // Step 2: Variable-length decode
    auto decoded = varLengthDecode(cipher, origSize);

    double elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeDone(decoded.size(), elapsed);
    return decoded;
}

/* ---- Reset ---- */

void TapirCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
