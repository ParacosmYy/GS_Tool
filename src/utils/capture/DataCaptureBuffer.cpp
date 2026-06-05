/**
 * @file DataCaptureBuffer.cpp
 * @brief 环形捕获缓冲区实现 — 连续/触发/单次模式
 */

#include "utils/capture/DataCaptureBuffer.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

/** @brief 构造函数 @param parent 父对象 */
DataCaptureBuffer::DataCaptureBuffer(QObject* parent)
    : QObject(parent)
    , m_writeIndex(0)
    , m_count(0)
    , m_captureMode(CaptureMode::Continuous)
    , m_isCapturing(false)
    , m_triggered(false)
    , m_triggerThreshold(0.0)
    , m_bufferSize(10000)
{
    m_buffer.resize(m_bufferSize);
}

/** @brief 开始捕获 */
void DataCaptureBuffer::startCapture()
{
    m_isCapturing = true;
    m_triggered   = false;
    m_writeIndex  = 0;
    m_count       = 0;
    m_buffer.fill(0.0);
    emit captureStarted();
}

/** @brief 停止捕获 */
void DataCaptureBuffer::stopCapture()
{
    m_isCapturing = false;
    m_triggered   = false;
    emit captureStopped();
}

/** @brief 写入一个采样值 @param value 数据值 */
void DataCaptureBuffer::addSample(double value)
{
    if (!m_isCapturing) return;

    switch (m_captureMode) {
    case CaptureMode::Continuous:
        writeSample(value);
        break;

    case CaptureMode::Triggered:
        /* 仅在已触发后采集 */
        if (m_triggered) {
            writeSample(value);
        }
        break;

    case CaptureMode::OneShot:
        /* 缓冲区未满时采集 */
        if (m_count < m_bufferSize) {
            writeSample(value);
        } else {
            /* 单次模式已满，自动停止 */
            m_isCapturing = false;
            emit captureStopped();
        }
        break;
    }
}

/** @brief 触发捕获 @param value 触发值 */
void DataCaptureBuffer::trigger(double value)
{
    if (value > m_triggerThreshold) {
        m_triggered = true;
        ++m_stats.totalTriggersFired;
        emit triggerFired();
    }
}

/** @brief 设置缓冲区大小 @param size 最大采样数 */
void DataCaptureBuffer::setBufferSize(int size)
{
    m_bufferSize = qMax(1, size);
    m_buffer.resize(m_bufferSize);
    m_writeIndex = 0;
    m_count      = 0;
}

/** @brief 设置捕获模式 @param mode 捕获模式 */
void DataCaptureBuffer::setCaptureMode(CaptureMode mode)
{
    m_captureMode = mode;
}

/** @brief 设置触发门限值 @param threshold 门限值 */
void DataCaptureBuffer::setTriggerThreshold(double threshold)
{
    m_triggerThreshold = threshold;
}

/** @brief 获取已捕获的数据(按写入顺序) @return 数据列表 */
QVector<double> DataCaptureBuffer::capturedData() const
{
    QVector<double> result;
    int n = qMin(m_count, m_bufferSize);
    result.reserve(n);
    if (m_count <= m_bufferSize) {
        /* 未环绕: 从0到writeIndex */
        for (int i = 0; i < m_writeIndex; ++i) {
            result.append(m_buffer[i]);
        }
    } else {
        /* 已环绕: 从writeIndex到末尾, 再从0到writeIndex-1 */
        for (int i = m_writeIndex; i < m_bufferSize; ++i) {
            result.append(m_buffer[i]);
        }
        for (int i = 0; i < m_writeIndex; ++i) {
            result.append(m_buffer[i]);
        }
    }
    return result;
}

/** @brief 导出已捕获数据到CSV @param path 文件路径 @return 是否成功 */
bool DataCaptureBuffer::exportToCsv(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    /* 写入CSV头部 */
    out << tr("Index") << ","
        << tr("Value") << ","
        << tr("Timestamp") << "\n";

    QVector<double> data = capturedData();
    qint64 baseTime = QDateTime::currentMSecsSinceEpoch() - data.size();
    for (int i = 0; i < data.size(); ++i) {
        out << i << "," << data[i] << ","
            << QDateTime::fromMSecsSinceEpoch(baseTime + i)
                      .toString(Qt::ISODateWithMs)
            << "\n";
    }
    file.close();
    return true;
}

/** @brief 当前缓冲区中的采样数 @return 采样数 */
int DataCaptureBuffer::sampleCount() const
{
    return qMin(m_count, m_bufferSize);
}

/** @brief 重置所有统计计数器 */
void DataCaptureBuffer::resetStatistics()
{
    m_stats = Stats{};
}

/** @brief 清空缓冲区(保留大小和模式) */
void DataCaptureBuffer::clearBuffer()
{
    m_writeIndex = 0;
    m_count      = 0;
    m_triggered  = false;
    m_buffer.fill(0.0);
}

/* ── 私有方法 ── */

/** @brief 写入单个采样到环形缓冲区 @param value 数据值 */
void DataCaptureBuffer::writeSample(double value)
{
    if (m_count >= m_bufferSize) {
        /* 环形覆盖: 旧数据丢失 */
        ++m_stats.totalOverflows;
        ++m_stats.samplesDropped;
        emit dataOverflow(1);
    }
    m_buffer[m_writeIndex] = value;
    m_writeIndex = (m_writeIndex + 1) % m_bufferSize;
    ++m_count;
    ++m_stats.totalSamplesCaptured;

    int usage = qMin(m_count, m_bufferSize);
    if (usage > m_stats.peakBufferSize) {
        m_stats.peakBufferSize = usage;
    }
}
