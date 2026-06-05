/**
 * @file SpdyCompressor.cpp
 * @brief SPDY头部压缩器实现
 */

#include "SpdyCompressor.h"
#include <QElapsedTimer>
#include <cstring>

SpdyCompressor::SpdyCompressor(int dynamicTableSize, QObject* parent)
    : QObject(parent)
    , m_maxTableSize(dynamicTableSize)
    , m_dynamicTableSize(0)
    , m_timeSum(0.0)
{
    /* 静态字典(常见HTTP头部) */
    m_staticTable[":method"] = 1;
    m_staticTable[":path"] = 3;
    m_staticTable[":scheme"] = 5;
    m_staticTable[":authority"] = 7;
    m_staticTable["accept"] = 10;
    m_staticTable["accept-encoding"] = 12;
    m_staticTable["accept-language"] = 14;
    m_staticTable["content-type"] = 16;
    m_staticTable["cache-control"] = 18;
    m_staticTable["cookie"] = 20;
    m_staticTable["host"] = 22;
    m_staticTable["user-agent"] = 24;
}

QByteArray SpdyCompressor::compress(const QMap<QString, QString>& headers)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    int originalSize = 0;

    for (auto it = headers.begin(); it != headers.end(); ++it) {
        const QString& name = it.key();
        const QString& value = it.value();
        originalSize += name.size() + value.size() + 2;

        /* 尝试静态表索引 */
        if (m_staticTable.contains(name)) {
            int idx = m_staticTable[name];
            result.append(encodeInteger(idx + 1, 6));
            result.append(encodeInteger(value.size(), 7));
            result.append(value.toUtf8());
            continue;
        }

        /* 尝试动态表 */
        bool found = false;
        for (int i = 0; i < m_dynamicTable.size(); ++i) {
            if (m_dynamicTable[i].first == name && m_dynamicTable[i].second == value) {
                int idx = m_staticTable.size() + i + 1;
                result.append(encodeInteger(idx, 6));
                found = true;
                break;
            }
        }

        if (!found) {
            /* 字面量编码 */
            result.append(static_cast<char>(0x00));
            result.append(encodeInteger(name.size(), 6));
            result.append(name.toUtf8());
            result.append(encodeInteger(value.size(), 7));
            result.append(value.toUtf8());

            /* 添加到动态表 */
            int entrySize = name.size() + value.size() + 32;
            while (m_dynamicTableSize + entrySize > m_maxTableSize && !m_dynamicTable.isEmpty()) {
                m_dynamicTableSize -= m_dynamicTable.last().first.size() +
                                       m_dynamicTable.last().second.size() + 32;
                m_dynamicTable.removeLast();
            }
            if (m_dynamicTableSize + entrySize <= m_maxTableSize) {
                m_dynamicTable.prepend({name, value});
                m_dynamicTableSize += entrySize;
            }
        }
    }

    m_stats.totalCompressed++;
    m_stats.totalBytesIn += originalSize;
    m_stats.totalBytesOut += result.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalCompressed + m_stats.totalDecompressed;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit compressionCompleted(originalSize, result.size());
    return result;
}

QMap<QString, QString> SpdyCompressor::decompress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QMap<QString, QString> headers;
    int offset = 0;

    while (offset < data.size()) {
        quint8 first = static_cast<quint8>(data[offset]);

        if (first & 0x80) {
            /* 索引头部 */
            auto [idx, consumed] = decodeInteger(data, offset, 6);
            idx -= 1;
            offset += consumed;

            /* 查找 */
            if (idx < m_staticTable.size()) {
                /* 静态表(简化: 只记录name) */
                for (auto it = m_staticTable.begin(); it != m_staticTable.end(); ++it) {
                    if (it.value() == idx) {
                        auto [vLen, vc] = decodeInteger(data, offset, 7);
                        offset += vc;
                        headers[it.key()] = QString::fromUtf8(data.mid(offset, vLen));
                        offset += vLen;
                        break;
                    }
                }
            } else {
                int dIdx = idx - m_staticTable.size();
                if (dIdx < m_dynamicTable.size()) {
                    headers[m_dynamicTable[dIdx].first] = m_dynamicTable[dIdx].second;
                }
            }
        } else {
            /* 字面量 */
            offset++;
            auto [nLen, nc] = decodeInteger(data, offset, 6);
            offset += nc;
            QString name = QString::fromUtf8(data.mid(offset, nLen));
            offset += nLen;

            auto [vLen, vc] = decodeInteger(data, offset, 7);
            offset += vc;
            QString value = QString::fromUtf8(data.mid(offset, vLen));
            offset += vLen;

            headers[name] = value;
        }
    }

    m_stats.totalDecompressed++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalCompressed + m_stats.totalDecompressed;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return headers;
}

QByteArray SpdyCompressor::encodeInteger(int value, int prefixBits)
{
    QByteArray result;
    int maxPrefix = (1 << prefixBits) - 1;

    if (value < maxPrefix) {
        result.append(static_cast<char>(value));
    } else {
        result.append(static_cast<char>(maxPrefix));
        value -= maxPrefix;
        while (value >= 128) {
            result.append(static_cast<char>((value & 0x7F) | 0x80));
            value >>= 7;
        }
        result.append(static_cast<char>(value));
    }
    return result;
}

QPair<int, int> SpdyCompressor::decodeInteger(const QByteArray& data, int offset,
                                                  int prefixBits)
{
    int maxPrefix = (1 << prefixBits) - 1;
    int value = static_cast<quint8>(data[offset]) & maxPrefix;
    int consumed = 1;

    if (value < maxPrefix) return {value, consumed};

    int m = 0;
    int total = value;
    while (offset + consumed < data.size()) {
        quint8 b = static_cast<quint8>(data[offset + consumed]);
        consumed++;
        total += (b & 0x7F) << m;
        m += 7;
        if (!(b & 0x80)) break;
    }
    return {total, consumed};
}

double SpdyCompressor::compressionRatio() const
{
    if (m_stats.totalBytesOut == 0) return 0.0;
    return static_cast<double>(m_stats.totalBytesOut) / m_stats.totalBytesIn;
}

int SpdyCompressor::dynamicTableSize() const { return m_dynamicTableSize; }

SpdyCompressor::Stats SpdyCompressor::stats() const { return m_stats; }

void SpdyCompressor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
