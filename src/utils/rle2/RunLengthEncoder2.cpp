/**
 * @file RunLengthEncoder2.cpp
 * @brief 增强型游程编码器实现
 */

#include "utils/rle2/RunLengthEncoder2.h"

#include <QElapsedTimer>

RunLengthEncoder2::RunLengthEncoder2(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QByteArray RunLengthEncoder2::encodeByteRun(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    int n = data.size();
    if (n == 0) return result;

    int i = 0;
    while (i < n) {
        /* 计算当前字节的重复次数 */
        char val = data[i];
        int run = 1;
        while (i + run < n && data[i + run] == val && run < 127) {
            ++run;
        }

        if (run >= 3) {
            /* 游程: 标记为负数计数值 */
            result.append(static_cast<char>(257 - run));   // (256-run) as unsigned byte
            result.append(val);
            i += run;
        } else {
            /* 字面量: 标记为正数计数值 */
            int litStart = i;
            int litLen = 0;
            while (i < n && litLen < 127) {
                /* 如果后面有>=3的重复，停止字面量 */
                int ahead = 1;
                while (i + ahead < n && data[i + ahead] == data[i] && ahead < 3) {
                    ++ahead;
                }
                if (ahead >= 3) break;
                ++i;
                ++litLen;
            }
            result.append(static_cast<char>(litLen - 1));
            result.append(data.mid(litStart, litLen));
        }
    }

    m_stats.totalEncodes++;
    m_stats.totalBytesIn += n;
    m_stats.totalBytesOut += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalEncodes, 1ULL);

    emit encoded(n, result.size());
    return result;
}

QByteArray RunLengthEncoder2::decodeByteRun(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    int i = 0;
    int n = data.size();

    while (i < n) {
        unsigned char header = static_cast<unsigned char>(data[i]);
        ++i;

        if (header <= 127) {
            /* 字面量: header+1个字节 */
            int count = header + 1;
            if (i + count > n) break;
            result.append(data.mid(i, count));
            i += count;
        } else {
            /* 游程: 重复 257-header 次 */
            int count = 257 - header;
            if (i >= n) break;
            result.append(QByteArray(count, data[i]));
            ++i;
        }
    }

    m_stats.totalDecodes++;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalEncodes + m_stats.totalDecodes, 1ULL);

    emit decoded(data.size(), result.size());
    return result;
}

QByteArray RunLengthEncoder2::encodeBitRun(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    if (data.isEmpty()) return result;

    /* 将字节数据展开为位流，然后对连续相同位计数 */
    QVector<int> bitRuns;
    int currentBit = -1;
    int currentRun = 0;

    for (int i = 0; i < data.size(); ++i) {
        unsigned char byte = static_cast<unsigned char>(data[i]);
        for (int b = 7; b >= 0; --b) {
            int bit = (byte >> b) & 1;
            if (currentBit == -1) {
                currentBit = bit;
                currentRun = 1;
            } else if (bit == currentBit && currentRun < 255) {
                ++currentRun;
            } else {
                bitRuns.append(currentBit);
                bitRuns.append(currentRun);
                currentBit = bit;
                currentRun = 1;
            }
        }
    }
    if (currentBit >= 0) {
        bitRuns.append(currentBit);
        bitRuns.append(currentRun);
    }

    /* 编码: 起始位 + [count] 序列 */
    result.append(static_cast<char>(currentBit >= 0 ? currentBit : 0));
    for (int i = 1; i < bitRuns.size(); i += 2) {
        result.append(static_cast<char>(bitRuns[i]));
    }

    m_stats.totalEncodes++;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalEncodes + m_stats.totalDecodes, 1ULL);

    emit encoded(data.size(), result.size());
    return result;
}

QByteArray RunLengthEncoder2::decodeBitRun(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    if (data.size() < 2) return result;

    int startBit = static_cast<unsigned char>(data[0]);
    int currentBit = startBit;
    int totalBits = 0;
    QByteArray bitBuf;

    for (int i = 1; i < data.size(); ++i) {
        int count = static_cast<unsigned char>(data[i]);
        for (int j = 0; j < count; ++j) {
            bitBuf.append(static_cast<char>(currentBit));
            ++totalBits;
        }
        currentBit = 1 - currentBit;
    }

    /* 将位流转回字节 */
    int bytes = totalBits / 8;
    for (int i = 0; i < bytes; ++i) {
        unsigned char byte = 0;
        for (int b = 0; b < 8; ++b) {
            int idx = i * 8 + b;
            if (idx < bitBuf.size() && bitBuf[idx] != 0) {
                byte |= (1 << (7 - b));
            }
        }
        result.append(static_cast<char>(byte));
    }

    m_stats.totalDecodes++;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalEncodes + m_stats.totalDecodes, 1ULL);

    emit decoded(data.size(), result.size());
    return result;
}

QByteArray RunLengthEncoder2::decodePackBits(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    int i = 0;
    int n = data.size();

    while (i < n) {
        signed char header = static_cast<signed char>(data[i]);
        ++i;

        if (header >= 0) {
            /* 字面量: header+1个字节直接拷贝 */
            int count = header + 1;
            if (i + count > n) break;
            result.append(data.mid(i, count));
            i += count;
        } else if (header > -128) {
            /* 游程: 下一个字节重复 1-header 次 */
            int count = 1 - header;
            if (i >= n) break;
            result.append(QByteArray(count, data[i]));
            ++i;
        }
        /* header == -128: 无操作 */
    }

    m_stats.totalDecodes++;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalEncodes + m_stats.totalDecodes, 1ULL);

    emit decoded(data.size(), result.size());
    return result;
}

QByteArray RunLengthEncoder2::encode(const QByteArray& data, Mode mode)
{
    switch (mode) {
    case Mode::ByteRun: return encodeByteRun(data);
    case Mode::BitRun:  return encodeBitRun(data);
    default: return encodeByteRun(data);
    }
}

QByteArray RunLengthEncoder2::decode(const QByteArray& data, Mode mode)
{
    switch (mode) {
    case Mode::ByteRun:  return decodeByteRun(data);
    case Mode::BitRun:   return decodeBitRun(data);
    case Mode::PackBits: return decodePackBits(data);
    default: return decodeByteRun(data);
    }
}

void RunLengthEncoder2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
