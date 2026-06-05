/**
 * @file WavReader.cpp
 * @brief WAV文件读取器实现
 */

#include "utils/wavreader/WavReader.h"

#include <QElapsedTimer>
#include <QDataStream>

WavReader::WavReader(QObject* parent)
    : QObject(parent), m_sampleRate(0), m_channels(0),
      m_bitsPerSample(0), m_dataOffset(0), m_dataSize(0),
      m_sampleCount(0), m_samplesRead(0), m_timeSum(0.0) {}

bool WavReader::open(const QString& filename)
{
    m_file.setFileName(filename);
    if (!m_file.open(QIODevice::ReadOnly)) return false;

    QDataStream in(&m_file);
    in.setByteOrder(QDataStream::LittleEndian);

    /* 检查RIFF头 */
    char riff[4];
    in.readRawData(riff, 4);
    if (std::memcmp(riff, "RIFF", 4) != 0) {
        m_file.close();
        return false;
    }

    quint32 fileSize;
    in >> fileSize;

    char wave[4];
    in.readRawData(wave, 4);
    if (std::memcmp(wave, "WAVE", 4) != 0) {
        m_file.close();
        return false;
    }

    /* 解析chunk */
    while (!in.atEnd()) {
        char chunkId[4];
        if (in.readRawData(chunkId, 4) != 4) break;
        quint32 chunkSize;
        in >> chunkSize;

        if (std::memcmp(chunkId, "fmt ", 4) == 0) {
            quint16 audioFormat, numChannels;
            quint32 sr, byteRate;
            quint16 blockAlign, bps;

            in >> audioFormat >> numChannels >> sr >> byteRate
               >> blockAlign >> bps;

            m_channels = numChannels;
            m_sampleRate = static_cast<int>(sr);
            m_bitsPerSample = bps;

            /* 跳过fmt chunk剩余数据 */
            if (chunkSize > 16)
                m_file.seek(m_file.pos() + chunkSize - 16);
        } else if (std::memcmp(chunkId, "data", 4) == 0) {
            m_dataOffset = m_file.pos();
            m_dataSize = chunkSize;
            int bytesPerSample = m_bitsPerSample / 8;
            m_sampleCount = (bytesPerSample > 0)
                            ? chunkSize / bytesPerSample : 0;
            m_samplesRead = 0;
            return true;
        } else {
            m_file.seek(m_file.pos() + chunkSize);
        }
    }

    m_file.close();
    return false;
}

double WavReader::readSample()
{
    QDataStream in(&m_file);
    in.setByteOrder(QDataStream::LittleEndian);

    if (m_bitsPerSample == 8) {
        quint8 val;
        in >> val;
        return (static_cast<double>(val) - 128.0) / 128.0;
    } else if (m_bitsPerSample == 16) {
        qint16 val;
        in >> val;
        return static_cast<double>(val) / 32768.0;
    } else if (m_bitsPerSample == 24) {
        quint8 b0, b1, b2;
        in >> b0 >> b1 >> b2;
        int val = b0 | (b1 << 8) | (b2 << 16);
        if (val & 0x800000) val |= 0xFF000000;
        return static_cast<double>(val) / 8388608.0;
    } else if (m_bitsPerSample == 32) {
        qint32 val;
        in >> val;
        return static_cast<double>(val) / 2147483648.0;
    }
    return 0.0;
}

QVector<double> WavReader::readAll()
{
    QElapsedTimer timer;
    timer.start();

    if (!m_file.isOpen()) return {};

    m_file.seek(m_dataOffset);
    QVector<double> result;
    result.reserve(static_cast<int>(m_sampleCount));

    for (quint64 i = 0; i < m_sampleCount; ++i)
        result.append(readSample());

    m_samplesRead = m_sampleCount;
    m_stats.totalSamplesRead += m_sampleCount;
    m_stats.totalFilesRead++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_stats.totalFilesRead > 0
        ? m_timeSum / m_stats.totalFilesRead : 0.0;

    emit fileRead(m_file.fileName(), result.size());
    return result;
}

QVector<double> WavReader::read(int count)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_file.isOpen()) return {};

    QVector<double> result;
    result.reserve(count);

    int toRead = qMin(static_cast<quint64>(count),
                      m_sampleCount - m_samplesRead);
    for (int i = 0; i < toRead; ++i)
        result.append(readSample());

    m_samplesRead += toRead;
    m_stats.totalSamplesRead += toRead;
    m_timeSum += timer.elapsed();

    return result;
}

void WavReader::close()
{
    if (m_file.isOpen())
        m_file.close();
}

void WavReader::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
