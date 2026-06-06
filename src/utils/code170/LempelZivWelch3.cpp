/**
 * @file LempelZivWelch3.cpp
 * @brief LempelZivWelch3 实现
 *
 * 实现LZW编解码：字典管理、变宽编码、提前重置、流式处理。
 */

#include "utils/code170/LempelZivWelch3.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

LempelZivWelch3::LempelZivWelch3(QObject *parent)
    : QObject(parent)
{
}

LempelZivWelch3::~LempelZivWelch3() = default;

/* ---- Configuration ---- */

void LempelZivWelch3::setMinCodeBits(int bits) { m_minBits = qBound(9, bits, 16); }
void LempelZivWelch3::setMaxCodeBits(int bits) { m_maxBits = qBound(m_minBits, bits, 16); }
void LempelZivWelch3::setEarlyResetEnabled(bool enabled) { m_earlyReset = enabled; }

/* ---- Dictionary init ---- */

void LempelZivWelch3::initDictionary()
{
    /* No explicit dictionary stored — encode/decode manage their own */
}

bool LempelZivWelch3::shouldReset(int dictSize) const
{
    int maxEntries = (1 << m_maxBits) - 1;
    return m_earlyReset && dictSize >= maxEntries;
}

/* ---- Encode ---- */

QVector<int> LempelZivWelch3::encode(const QByteArray& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return QVector<int>();

    QVector<int> output;
    int nextCode = 256;  /* Codes 0-255 for single bytes */
    int currentBits = m_minBits;
    int maxCode = (1 << currentBits) - 1;
    int resetCount = 0;

    /* Dictionary: maps (prefix_code, byte) -> new_code */
    /* Use flat hash table via simple open addressing */
    struct Entry {
        int prefix = -1;
        quint8 byte = 0;
        int code = -1;
    };

    int tableSize = 1 << 16;
    QVector<Entry> table(tableSize);
    int entryCount = 0;

    auto hashFunc = [](int prefix, quint8 b) -> int {
        return ((prefix << 8) ^ b) & 0xFFFF;
    };

    auto findEntry = [&](int prefix, quint8 b) -> int {
        int idx = hashFunc(prefix, b);
        for (int probe = 0; probe < tableSize; ++probe) {
            int pos = (idx + probe) % tableSize;
            if (table[pos].code == -1) return -1;
            if (table[pos].prefix == prefix && table[pos].byte == b)
                return table[pos].code;
        }
        return -1;
    };

    auto addEntry = [&](int prefix, quint8 b, int code) {
        int idx = hashFunc(prefix, b);
        for (int probe = 0; probe < tableSize; ++probe) {
            int pos = (idx + probe) % tableSize;
            if (table[pos].code == -1) {
                table[pos] = {prefix, b, code};
                entryCount++;
                return;
            }
        }
    };

    int w = static_cast<quint8>(input[0]);

    for (int i = 1; i < n; ++i) {
        quint8 c = static_cast<quint8>(input[i]);
        int wc = findEntry(w, c);

        if (wc >= 0) {
            w = wc;
        } else {
            output.append(w);

            /* Check if dictionary needs reset */
            if (shouldReset(nextCode)) {
                nextCode = 256;
                currentBits = m_minBits;
                maxCode = (1 << currentBits) - 1;
                table.fill({});
                entryCount = 0;
                resetCount++;
            } else {
                if (nextCode <= maxCode) {
                    addEntry(w, c, nextCode);
                    nextCode++;
                }
                if (nextCode > maxCode && currentBits < m_maxBits) {
                    currentBits++;
                    maxCode = (1 << currentBits) - 1;
                }
            }

            w = static_cast<int>(c);
        }
    }
    output.append(w);

    m_stats.totalEncodes++;
    m_stats.lastDictSize = nextCode;
    m_stats.dictionaryResets = resetCount;
    int compressedBits = output.size() * currentBits;
    double ratio = (n > 0) ? static_cast<double>(compressedBits) / (n * 8) : 0.0;
    m_stats.avgCompressionRatio = (m_stats.totalEncodes > 1)
        ? (m_stats.avgCompressionRatio * (m_stats.totalEncodes - 1) + ratio) / m_stats.totalEncodes
        : ratio;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(n, output.size());
    if (resetCount > 0) emit dictionaryReset(resetCount);
    return output;
}

/* ---- Decode ---- */

QByteArray LempelZivWelch3::decode(const QVector<int>& codes)
{
    QElapsedTimer timer;
    timer.start();

    if (codes.isEmpty()) return QByteArray();

    /* Build string table: code -> byte sequence */
    QVector<QByteArray> table(1 << m_maxBits);
    for (int i = 0; i < 256; ++i)
        table[i] = QByteArray(1, static_cast<char>(i));

    int nextCode = 256;
    int currentBits = m_minBits;
    QByteArray output;

    int prevCode = codes[0];
    if (prevCode < 0 || prevCode >= nextCode) return output;
    output.append(table[prevCode]);

    for (int i = 1; i < codes.size(); ++i) {
        int code = codes[i];

        QByteArray entry;
        if (code < nextCode) {
            entry = table[code];
        } else if (code == nextCode) {
            /* Special case: code not in table yet */
            entry = table[prevCode] + table[prevCode][0];
        } else {
            break; /* Invalid code */
        }

        output.append(entry);

        if (nextCode < (1 << m_maxBits)) {
            table[nextCode] = table[prevCode] + entry[0];
            nextCode++;
            if (nextCode > (1 << currentBits) - 1 && currentBits < m_maxBits)
                currentBits++;
        }

        /* Early reset handling */
        if (m_earlyReset && nextCode >= (1 << m_maxBits) - 1) {
            nextCode = 256;
            currentBits = m_minBits;
        }

        prevCode = code;
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(output.size());
    return output;
}

/* ---- Byte-packed encode/decode ---- */

QByteArray LempelZivWelch3::encodeToBytes(const QByteArray& input)
{
    QVector<int> codes = encode(input);
    if (codes.isEmpty()) return QByteArray();

    /* Pack codes using current bit width */
    int bits = m_minBits;
    QByteArray packed;
    quint32 buffer = 0;
    int bufBits = 0;

    for (int code : codes) {
        buffer |= (static_cast<quint32>(code) << bufBits);
        bufBits += bits;

        while (bufBits >= 8) {
            packed.append(static_cast<char>(buffer & 0xFF));
            buffer >>= 8;
            bufBits -= 8;
        }
    }
    if (bufBits > 0)
        packed.append(static_cast<char>(buffer & 0xFF));

    return packed;
}

QByteArray LempelZivWelch3::decodeFromBytes(const QByteArray& packed)
{
    int bits = m_minBits;
    QVector<int> codes;
    quint32 buffer = 0;
    int bufBits = 0;
    int maxCode = (1 << bits) - 1;
    int nextCode = 256;

    for (int i = 0; i < packed.size(); ++i) {
        buffer |= (static_cast<quint32>(static_cast<quint8>(packed[i])) << bufBits);
        bufBits += 8;

        while (bufBits >= bits) {
            int code = buffer & ((1 << bits) - 1);
            buffer >>= bits;
            bufBits -= bits;
            codes.append(code);

            nextCode++;
            if (nextCode > maxCode && bits < m_maxBits) {
                bits++;
                maxCode = (1 << bits) - 1;
            }
        }
    }

    return decode(codes);
}

/* ---- Statistics ---- */

void LempelZivWelch3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
