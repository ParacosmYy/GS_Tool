/**
 * @file RleEncoder.cpp
 * @brief RLE游程编码实现
 */

#include "RleEncoder.h"
#include <QElapsedTimer>

RleEncoder::RleEncoder(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QByteArray RleEncoder::encode(const QByteArray& data, Mode mode) const
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;

    if (data.isEmpty()) {
        m_stats.totalEncoded++;
        m_timeSum += timer.elapsed();
        return result;
    }

    switch (mode) {
    case Classic: {
        int i = 0;
        while (i < data.size()) {
            char val = data[i];
            int run = 1;
            while (i + run < data.size() && data[i + run] == val && run < 255)
                run++;

            result.append(static_cast<char>(run));
            result.append(val);
            i += run;
        }
        break;
    }
    case PackBits: {
        int i = 0;
        while (i < data.size()) {
            /* 检查是否为重复序列 */
            if (i + 1 < data.size() && data[i] == data[i + 1]) {
                int run = 1;
                while (i + run < data.size() && data[i + run] == data[i] && run < 128)
                    run++;

                result.append(static_cast<char>(257 - run)); /* 控制字: 128~255 */
                result.append(data[i]);
                i += run;
            } else {
                /* 非重复序列 */
                int start = i;
                int len = 1;
                while (i + len < data.size() && len < 128) {
                    if (i + len + 1 < data.size() && data[i + len] == data[i + len + 1])
                        break;
                    len++;
                }

                result.append(static_cast<char>(len - 1)); /* 控制字: 0~127 */
                result.append(data.mid(i, len));
                i += len;
            }
        }
        break;
    }
    case HeaderLess: {
        int i = 0;
        while (i < data.size()) {
            char val = data[i];
            int run = 1;
            while (i + run < data.size() && data[i + run] == val && run < 65535)
                run++;

            result.append(static_cast<char>((run >> 8) & 0xFF));
            result.append(static_cast<char>(run & 0xFF));
            result.append(val);
            i += run;
        }
        break;
    }
    }

    m_stats.totalEncoded++;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += result.size();
    if (m_stats.totalBytesIn > 0)
        m_stats.avgCompressionRatio = static_cast<double>(m_stats.totalBytesOut) / m_stats.totalBytesIn;

    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit encodeCompleted(data.size(), result.size(),
        data.size() > 0 ? static_cast<double>(result.size()) / data.size() : 0.0);

    return result;
}

QByteArray RleEncoder::decode(const QByteArray& data, Mode mode) const
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;

    switch (mode) {
    case Classic: {
        int i = 0;
        while (i + 1 < data.size()) {
            int count = static_cast<quint8>(data[i]);
            char val = data[i + 1];
            result.append(QByteArray(count, val));
            i += 2;
        }
        break;
    }
    case PackBits: {
        int i = 0;
        while (i < data.size()) {
            quint8 ctrl = static_cast<quint8>(data[i]);
            i++;

            if (ctrl > 127) {
                /* 重复: (257 - ctrl)次 */
                int count = 257 - ctrl;
                if (i < data.size()) {
                    result.append(QByteArray(count, data[i]));
                    i++;
                }
            } else {
                /* 非重复: (ctrl + 1)字节 */
                int count = ctrl + 1;
                if (i + count <= data.size()) {
                    result.append(data.mid(i, count));
                    i += count;
                }
            }
        }
        break;
    }
    case HeaderLess: {
        int i = 0;
        while (i + 2 < data.size()) {
            int count = (static_cast<quint8>(data[i]) << 8) | static_cast<quint8>(data[i + 1]);
            char val = data[i + 2];
            result.append(QByteArray(count, val));
            i += 3;
        }
        break;
    }
    }

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

QPair<QVector<double>, QVector<int>> RleEncoder::encodeValues(
    const QVector<double>& values) const
{
    QVector<double> vals;
    QVector<int> counts;

    if (values.isEmpty()) return {vals, counts};

    double current = values[0];
    int count = 1;

    for (int i = 1; i < values.size(); ++i) {
        if (values[i] == current) {
            count++;
        } else {
            vals.append(current);
            counts.append(count);
            current = values[i];
            count = 1;
        }
    }
    vals.append(current);
    counts.append(count);

    return {vals, counts};
}

RleEncoder::Stats RleEncoder::stats() const { return m_stats; }

void RleEncoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
