/**
 * @file WavWriter.cpp
 * @brief WAV文件写入器实现
 */

#include "utils/wavwriter/WavWriter.h"

#include <QElapsedTimer>
#include <QDataStream>
#include <cstring>

WavWriter::WavWriter(QObject* parent)
    : QObject(parent), m_sampleRate(0), m_channels(0),
      m_bitsPerSample(0), m_dataSize(0), m_timeSum(0.0) {}

bool WavWriter::open(const QString& filename, int sampleRate,
                     int channels, int bitsPerSample)
{
    m_file.setFileName(filename);
    if (!m_file.open(QIODevice::WriteOnly))
        return false;

    m_sampleRate = sampleRate;
    m_channels = channels;
    m_bitsPerSample = bitsPerSample;
    m_dataSize = 0;

    writeHeader();
    return true;
}

void WavWriter::writeHeader()
{
    QDataStream out(&m_file);
    out.setByteOrder(QDataStream::LittleEndian);

    /* RIFF header */
    out.writeRawData("RIFF", 4);
    out << quint32(36); /* placeholder file size */
    out.writeRawData("WAVE", 4);

    /* fmt chunk */
    out.writeRawData("fmt ", 4);
    out << quint32(16);
    out << quint16(1); /* PCM format */
    out << quint16(m_channels);
    out << quint32(m_sampleRate);
    int blockAlign = m_channels * m_bitsPerSample / 8;
    out << quint32(m_sampleRate * blockAlign);
    out << quint16(blockAlign);
    out << quint16(m_bitsPerSample);

    /* data chunk header */
    out.writeRawData("data", 4);
    out << quint32(0); /* placeholder data size */
}

bool WavWriter::write(const QVector<double>& samples)
{
    if (!m_file.isOpen()) return false;

    QElapsedTimer timer;
    timer.start();

    QDataStream out(&m_file);
    out.setByteOrder(QDataStream::LittleEndian);

    for (double s : samples) {
        double clamped = qBound(-1.0, s, 1.0);
        if (m_bitsPerSample == 8) {
            out << quint8(static_cast<int>((clamped + 1.0) * 127.5));
        } else if (m_bitsPerSample == 16) {
            out << qint16(static_cast<int>(clamped * 32767.0));
        } else if (m_bitsPerSample == 24) {
            int val = static_cast<int>(clamped * 8388607.0);
            out << quint8(val & 0xFF);
            out << quint8((val >> 8) & 0xFF);
            out << quint8((val >> 16) & 0xFF);
        } else if (m_bitsPerSample == 32) {
            out << qint32(static_cast<long long>(clamped * 2147483647.0));
        }
    }
    m_dataSize += samples.size() * (m_bitsPerSample / 8);

    m_stats.totalSamplesWritten += samples.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalFilesWritten + m_stats.totalSamplesWritten > 0)
        ? m_timeSum / (m_stats.totalFilesWritten + 1) : 0.0;

    return true;
}

bool WavWriter::writeInt(const QVector<int>& samples)
{
    if (!m_file.isOpen()) return false;

    QElapsedTimer timer;
    timer.start();

    QDataStream out(&m_file);
    out.setByteOrder(QDataStream::LittleEndian);

    for (int s : samples) {
        if (m_bitsPerSample == 8) {
            out << quint8(s);
        } else if (m_bitsPerSample == 16) {
            out << qint16(s);
        } else if (m_bitsPerSample == 24) {
            out << quint8(s & 0xFF);
            out << quint8((s >> 8) & 0xFF);
            out << quint8((s >> 16) & 0xFF);
        } else if (m_bitsPerSample == 32) {
            out << qint32(s);
        }
    }
    m_dataSize += samples.size() * (m_bitsPerSample / 8);

    m_stats.totalSamplesWritten += samples.size();
    m_timeSum += timer.elapsed();

    return true;
}

void WavWriter::close()
{
    if (!m_file.isOpen()) return;

    updateHeader();
    m_file.close();

    m_stats.totalFilesWritten++;
    emit fileWritten(m_file.fileName(),
                     static_cast<int>(m_stats.totalSamplesWritten));
}

void WavWriter::updateHeader()
{
    m_file.seek(4);
    QDataStream out(&m_file);
    out.setByteOrder(QDataStream::LittleEndian);
    out << quint32(36 + m_dataSize);

    m_file.seek(40);
    out << quint32(m_dataSize);
}

void WavWriter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
