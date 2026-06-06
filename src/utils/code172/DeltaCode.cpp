/**
 * @file DeltaCode.cpp
 * @brief DeltaCode 实现
 *
 * 实现Delta编解码：XOR差分和PCM差分模式，支持8/16/32位字长。
 */

#include "utils/code172/DeltaCode.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

DeltaCode::DeltaCode(QObject *parent)
    : QObject(parent)
{
}

DeltaCode::~DeltaCode() = default;

/* ---- Configuration ---- */

void DeltaCode::setMode(DeltaMode mode) { m_mode = mode; }
void DeltaCode::setWordSize(WordSize ws) { m_wordSize = ws; }

/* ---- 8-bit XOR delta ---- */

QByteArray DeltaCode::encodeXor8(const QByteArray& input)
{
    int n = input.size();
    if (n == 0) return {};
    QByteArray out(n, Qt::Uninitialized);
    out[0] = input[0];
    for (int i = 1; i < n; ++i)
        out[i] = input[i] ^ input[i - 1];
    return out;
}

QByteArray DeltaCode::decodeXor8(const QByteArray& input)
{
    int n = input.size();
    if (n == 0) return {};
    QByteArray out(n, Qt::Uninitialized);
    out[0] = input[0];
    for (int i = 1; i < n; ++i)
        out[i] = input[i] ^ out[i - 1];
    return out;
}

/* ---- 8-bit PCM delta ---- */

QByteArray DeltaCode::encodePcm8(const QByteArray& input)
{
    int n = input.size();
    if (n == 0) return {};
    QByteArray out(n, Qt::Uninitialized);
    out[0] = input[0];
    for (int i = 1; i < n; ++i) {
        int delta = static_cast<quint8>(input[i]) - static_cast<quint8>(input[i - 1]);
        out[i] = static_cast<char>(delta);
    }
    return out;
}

QByteArray DeltaCode::decodePcm8(const QByteArray& input)
{
    int n = input.size();
    if (n == 0) return {};
    QByteArray out(n, Qt::Uninitialized);
    out[0] = input[0];
    for (int i = 1; i < n; ++i) {
        int val = static_cast<quint8>(out[i - 1]) + static_cast<qint8>(input[i]);
        out[i] = static_cast<char>(val & 0xFF);
    }
    return out;
}

/* ---- 16-bit XOR delta ---- */

QByteArray DeltaCode::encodeXor16(const QByteArray& input)
{
    int n = input.size() / 2;
    if (n == 0) return {};
    QByteArray out(n * 2, Qt::Uninitialized);
    /* Copy first sample */
    out[0] = input[0]; out[1] = input[1];
    for (int i = 1; i < n; ++i) {
        quint16 cur = static_cast<quint16>((static_cast<quint8>(input[2 * i]) << 8) |
                                            static_cast<quint8>(input[2 * i + 1]));
        quint16 prev = static_cast<quint16>((static_cast<quint8>(input[2 * (i - 1)]) << 8) |
                                             static_cast<quint8>(input[2 * (i - 1) + 1]));
        quint16 delta = cur ^ prev;
        out[2 * i] = static_cast<char>((delta >> 8) & 0xFF);
        out[2 * i + 1] = static_cast<char>(delta & 0xFF);
    }
    return out;
}

QByteArray DeltaCode::decodeXor16(const QByteArray& input)
{
    int n = input.size() / 2;
    if (n == 0) return {};
    QByteArray out(n * 2, Qt::Uninitialized);
    out[0] = input[0]; out[1] = input[1];
    for (int i = 1; i < n; ++i) {
        quint16 delta = static_cast<quint16>((static_cast<quint8>(input[2 * i]) << 8) |
                                              static_cast<quint8>(input[2 * i + 1]));
        quint16 prev = static_cast<quint16>((static_cast<quint8>(out[2 * (i - 1)]) << 8) |
                                             static_cast<quint8>(out[2 * (i - 1) + 1]));
        quint16 val = delta ^ prev;
        out[2 * i] = static_cast<char>((val >> 8) & 0xFF);
        out[2 * i + 1] = static_cast<char>(val & 0xFF);
    }
    return out;
}

/* ---- 16-bit PCM delta ---- */

QByteArray DeltaCode::encodePcm16(const QByteArray& input)
{
    int n = input.size() / 2;
    if (n == 0) return {};
    QByteArray out(n * 2, Qt::Uninitialized);
    out[0] = input[0]; out[1] = input[1];
    for (int i = 1; i < n; ++i) {
        qint16 cur = static_cast<qint16>((static_cast<quint8>(input[2 * i]) << 8) |
                                          static_cast<quint8>(input[2 * i + 1]));
        qint16 prev = static_cast<qint16>((static_cast<quint8>(input[2 * (i - 1)]) << 8) |
                                           static_cast<quint8>(input[2 * (i - 1) + 1]));
        qint16 delta = cur - prev;
        out[2 * i] = static_cast<char>((delta >> 8) & 0xFF);
        out[2 * i + 1] = static_cast<char>(delta & 0xFF);
    }
    return out;
}

QByteArray DeltaCode::decodePcm16(const QByteArray& input)
{
    int n = input.size() / 2;
    if (n == 0) return {};
    QByteArray out(n * 2, Qt::Uninitialized);
    out[0] = input[0]; out[1] = input[1];
    for (int i = 1; i < n; ++i) {
        qint16 delta = static_cast<qint16>((static_cast<quint8>(input[2 * i]) << 8) |
                                            static_cast<quint8>(input[2 * i + 1]));
        qint16 prev = static_cast<qint16>((static_cast<quint8>(out[2 * (i - 1)]) << 8) |
                                           static_cast<quint8>(out[2 * (i - 1) + 1]));
        qint16 val = prev + delta;
        out[2 * i] = static_cast<char>((val >> 8) & 0xFF);
        out[2 * i + 1] = static_cast<char>(val & 0xFF);
    }
    return out;
}

/* ---- 32-bit XOR delta ---- */

QByteArray DeltaCode::encodeXor32(const QByteArray& input)
{
    int n = input.size() / 4;
    if (n == 0) return {};
    QByteArray out(n * 4, Qt::Uninitialized);
    for (int b = 0; b < 4; ++b) out[b] = input[b];
    for (int i = 1; i < n; ++i) {
        quint32 cur = 0, prev = 0;
        for (int b = 0; b < 4; ++b) {
            cur |= static_cast<quint32>(static_cast<quint8>(input[4 * i + b])) << (24 - 8 * b);
            prev |= static_cast<quint32>(static_cast<quint8>(input[4 * (i - 1) + b])) << (24 - 8 * b);
        }
        quint32 delta = cur ^ prev;
        for (int b = 0; b < 4; ++b)
            out[4 * i + b] = static_cast<char>((delta >> (24 - 8 * b)) & 0xFF);
    }
    return out;
}

QByteArray DeltaCode::decodeXor32(const QByteArray& input)
{
    int n = input.size() / 4;
    if (n == 0) return {};
    QByteArray out(n * 4, Qt::Uninitialized);
    for (int b = 0; b < 4; ++b) out[b] = input[b];
    for (int i = 1; i < n; ++i) {
        quint32 delta = 0, prev = 0;
        for (int b = 0; b < 4; ++b) {
            delta |= static_cast<quint32>(static_cast<quint8>(input[4 * i + b])) << (24 - 8 * b);
            prev |= static_cast<quint32>(static_cast<quint8>(out[4 * (i - 1) + b])) << (24 - 8 * b);
        }
        quint32 val = delta ^ prev;
        for (int b = 0; b < 4; ++b)
            out[4 * i + b] = static_cast<char>((val >> (24 - 8 * b)) & 0xFF);
    }
    return out;
}

/* ---- 32-bit PCM delta ---- */

QByteArray DeltaCode::encodePcm32(const QByteArray& input)
{
    int n = input.size() / 4;
    if (n == 0) return {};
    QByteArray out(n * 4, Qt::Uninitialized);
    for (int b = 0; b < 4; ++b) out[b] = input[b];
    for (int i = 1; i < n; ++i) {
        qint32 cur = 0, prev = 0;
        for (int b = 0; b < 4; ++b) {
            cur |= static_cast<qint32>(static_cast<quint8>(input[4 * i + b])) << (24 - 8 * b);
            prev |= static_cast<qint32>(static_cast<quint8>(input[4 * (i - 1) + b])) << (24 - 8 * b);
        }
        qint32 delta = cur - prev;
        for (int b = 0; b < 4; ++b)
            out[4 * i + b] = static_cast<char>((delta >> (24 - 8 * b)) & 0xFF);
    }
    return out;
}

QByteArray DeltaCode::decodePcm32(const QByteArray& input)
{
    int n = input.size() / 4;
    if (n == 0) return {};
    QByteArray out(n * 4, Qt::Uninitialized);
    for (int b = 0; b < 4; ++b) out[b] = input[b];
    for (int i = 1; i < n; ++i) {
        qint32 delta = 0, prev = 0;
        for (int b = 0; b < 4; ++b) {
            delta |= static_cast<qint32>(static_cast<quint8>(input[4 * i + b])) << (24 - 8 * b);
            prev |= static_cast<qint32>(static_cast<quint8>(out[4 * (i - 1) + b])) << (24 - 8 * b);
        }
        qint32 val = prev + delta;
        for (int b = 0; b < 4; ++b)
            out[4 * i + b] = static_cast<char>((val >> (24 - 8 * b)) & 0xFF);
    }
    return out;
}

/* ---- Encode dispatch ---- */

QByteArray DeltaCode::encode(const QByteArray& input)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    if (m_mode == XorDelta) {
        switch (m_wordSize) {
        case Bits16: result = encodeXor16(input); break;
        case Bits32: result = encodeXor32(input); break;
        default: result = encodeXor8(input); break;
        }
    } else {
        switch (m_wordSize) {
        case Bits16: result = encodePcm16(input); break;
        case Bits32: result = encodePcm32(input); break;
        default: result = encodePcm8(input); break;
        }
    }

    m_stats.totalEncodes++;
    m_stats.totalBytesIn += input.size();
    m_stats.totalBytesOut += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(input.size(), result.size());
    return result;
}

/* ---- Decode dispatch ---- */

QByteArray DeltaCode::decode(const QByteArray& input)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    if (m_mode == XorDelta) {
        switch (m_wordSize) {
        case Bits16: result = decodeXor16(input); break;
        case Bits32: result = decodeXor32(input); break;
        default: result = decodeXor8(input); break;
        }
    } else {
        switch (m_wordSize) {
        case Bits16: result = decodePcm16(input); break;
        case Bits32: result = decodePcm32(input); break;
        default: result = decodePcm8(input); break;
        }
    }

    m_stats.totalDecodes++;
    m_stats.totalBytesIn += input.size();
    m_stats.totalBytesOut += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(input.size(), result.size());
    return result;
}

/* ---- Statistics ---- */

void DeltaCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
