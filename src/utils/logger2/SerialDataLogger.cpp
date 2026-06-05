/**
 * @file SerialDataLogger.cpp
 * @brief 高级串口数据日志记录器 -- 核心: 构造/启停/数据记录/flush/配置
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 轮转逻辑见 @see SerialDataLoggerRotation.cpp
 * 格式导出见 @see SerialDataLoggerExport.cpp
 * 统计查询见 @see SerialDataLoggerStats.cpp
 */

#include "utils/logger2/SerialDataLogger.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

// ──────────────────────────────────────────────
// 构造与析构
// ──────────────────────────────────────────────

SerialDataLogger::SerialDataLogger(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SerialDataLogger"));

    // 定时 flush: 每秒将缓冲区写入磁盘
    m_flushTimer = new QTimer(this);
    m_flushTimer->setInterval(1000);
    connect(m_flushTimer, &QTimer::timeout, this, &SerialDataLogger::onFlushTimer);

    // 定时轮转检查: 每 30 秒检查大小/时间轮转条件
    m_rotationTimer = new QTimer(this);
    m_rotationTimer->setInterval(30000);
    connect(m_rotationTimer, &QTimer::timeout, this, &SerialDataLogger::onRotationCheck);
}

SerialDataLogger::~SerialDataLogger()
{
    if (m_logging) {
        stopLogging();
    }
}

// ──────────────────────────────────────────────
// 日志控制
// ──────────────────────────────────────────────

bool SerialDataLogger::startLogging(const QString& dirPath,
                                    const QString& pattern,
                                    LogFormat format)
{
    if (m_logging) {
        stopLogging();
    }

    m_logDirPath = dirPath;
    m_filePattern = pattern;
    m_format = format;

    // 确保日志目录存在
    m_logDir.setPath(dirPath);
    if (!m_logDir.exists()) {
        if (!m_logDir.mkpath(QStringLiteral("."))) {
            emit error(tr("无法创建日志目录: %1").arg(dirPath));
            return false;
        }
    }

    const QString filePath = resolvePattern(pattern);
    if (!openLogFile(filePath)) {
        return false;
    }

    m_startEpoch = QDateTime::currentMSecsSinceEpoch();
    m_rotationBaseEpoch = m_startEpoch;
    m_logging = true;
    m_paused = false;

    m_flushTimer->start();
    m_rotationTimer->start();

    emit logStarted();
    return true;
}

void SerialDataLogger::stopLogging()
{
    if (!m_logging) {
        return;
    }

    m_flushTimer->stop();
    m_rotationTimer->stop();

    flush();
    closeLogFile();

    m_logging = false;
    m_paused = false;

    emit logStopped();
}

void SerialDataLogger::pauseLogging()
{
    if (m_logging && !m_paused) {
        m_paused = true;
    }
}

void SerialDataLogger::resumeLogging()
{
    if (m_logging && m_paused) {
        m_paused = false;
    }
}

bool SerialDataLogger::isLogging() const { return m_logging; }
bool SerialDataLogger::isPaused() const  { return m_paused; }

// ──────────────────────────────────────────────
// 数据写入
// ──────────────────────────────────────────────

void SerialDataLogger::logData(const QByteArray& data, const QString& direction)
{
    if (!m_logging || m_paused || data.isEmpty()) {
        return;
    }

    const QByteArray record = formatRecord(data, direction);
    m_buffer.append(record);
    ++m_totalRecords;
    m_totalBytesLogged += static_cast<quint64>(data.size());

    // 缓冲区超限时自动 flush
    if (static_cast<quint64>(m_buffer.size()) >= m_bufferLimit) {
        flush();
    }
}

void SerialDataLogger::flush()
{
    if (m_buffer.isEmpty() || !m_file) {
        return;
    }

    writeToFile(m_buffer);
    m_buffer.clear();

    // 检查文件大小轮转
    if (m_maxFileSize > 0 && m_currentFileSize >= m_maxFileSize) {
        performRotation();
    }
}

// ──────────────────────────────────────────────
// 配置接口
// ──────────────────────────────────────────────

void SerialDataLogger::setMaxFileSize(quint64 bytes) { m_maxFileSize = bytes; }
quint64 SerialDataLogger::maxFileSize() const { return m_maxFileSize; }
void SerialDataLogger::setMaxFiles(int count) { m_maxFiles = count; }
void SerialDataLogger::setRotationPeriod(RotationPeriod p) { m_rotationPeriod = p; }
void SerialDataLogger::setTimestampFormat(TimestampFormat f) { m_timestampFormat = f; }
void SerialDataLogger::setCustomTimestampFormat(const QString& fmt) { m_customTimestampFormat = fmt; }
void SerialDataLogger::setBufferSize(quint64 bytes) { m_bufferLimit = qMax<quint64>(bytes, 1024); }

// ──────────────────────────────────────────────
// 查询接口
// ──────────────────────────────────────────────

QString SerialDataLogger::getCurrentLogFile() const
{
    return m_currentFilePath;
}

QStringList SerialDataLogger::getLogFiles() const
{
    QStringList result;
    if (!m_logDir.exists()) {
        return result;
    }

    const QStringList filters = {
        QStringLiteral("serial_*.log"),
        QStringLiteral("serial_*.csv"),
        QStringLiteral("serial_*.json"),
        QStringLiteral("serial_*.hex"),
        QStringLiteral("serial_*.txt"),
        QStringLiteral("serial_*.pcap"),
        QStringLiteral("serial_*.raw"),
        QStringLiteral("serial_*.log.gz"),
        QStringLiteral("serial_*.csv.gz"),
        QStringLiteral("serial_*.hex.gz")
    };

    const QFileInfoList entries = m_logDir.entryInfoList(
        filters, QDir::Files, QDir::Time | QDir::Reversed);

    for (const QFileInfo& fi : entries) {
        result.append(fi.absoluteFilePath());
    }
    return result;
}

// ──────────────────────────────────────────────
// 文件名占位符解析
// ──────────────────────────────────────────────

QString SerialDataLogger::resolvePattern(const QString& pattern) const
{
    const QDateTime now = QDateTime::currentDateTime();
    QString result = pattern;

    result.replace(QStringLiteral("{date}"), now.toString(QStringLiteral("yyyyMMdd")));
    result.replace(QStringLiteral("{time}"), now.toString(QStringLiteral("HHmmss")));
    result.replace(QStringLiteral("{index}"),
                   QString::number(static_cast<int>(m_totalFilesCreated + 1)));

    QString ext;
    switch (m_format) {
    case LogFormat::Raw:  ext = QStringLiteral("raw"); break;
    case LogFormat::Hex:  ext = QStringLiteral("hex"); break;
    case LogFormat::Ascii: ext = QStringLiteral("txt"); break;
    case LogFormat::Csv:  ext = QStringLiteral("csv"); break;
    case LogFormat::Json: ext = QStringLiteral("json"); break;
    case LogFormat::Pcap: ext = QStringLiteral("pcap"); break;
    }
    result += QStringLiteral(".") + ext;

    return m_logDir.filePath(result);
}

// ──────────────────────────────────────────────
// 文件打开/关闭
// ──────────────────────────────────────────────

bool SerialDataLogger::openLogFile(const QString& filePath)
{
    m_file = new QFile(filePath, this);
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    if (m_format != LogFormat::Raw && m_format != LogFormat::Pcap) {
        mode |= QIODevice::Text;
    }

    if (!m_file->open(mode)) {
        emit error(tr("无法打开日志文件: %1").arg(filePath));
        delete m_file;
        m_file = nullptr;
        return false;
    }

    m_currentFilePath = filePath;
    m_currentFileSize = 0;
    ++m_totalFilesCreated;

    // 写入格式头
    if (m_format == LogFormat::Pcap) {
        writeToFile(buildPcapGlobalHeader());
    } else if (m_format == LogFormat::Json) {
        writeToFile(QByteArray("{\"records\":[\n"));
    } else if (m_format == LogFormat::Csv) {
        writeToFile(QByteArray("timestamp,direction,size,data_hex,data_ascii\n"));
    }

    return true;
}

void SerialDataLogger::closeLogFile()
{
    if (!m_file) {
        return;
    }

    // 写入格式尾
    if (m_format == LogFormat::Json) {
        m_file->write(QByteArray("\n]}\n"));
    }

    m_file->close();
    m_file->deleteLater();
    m_file = nullptr;
}

// ──────────────────────────────────────────────
// 时间戳格式化
// ──────────────────────────────────────────────

QString SerialDataLogger::formatTimestamp() const
{
    switch (m_timestampFormat) {
    case TimestampFormat::None:
        return QString();
    case TimestampFormat::Epoch:
        return QString::number(QDateTime::currentMSecsSinceEpoch());
    case TimestampFormat::ISO8601:
        return QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    case TimestampFormat::Relative: {
        const qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_startEpoch;
        return QString::number(elapsed);
    }
    case TimestampFormat::Custom:
        return QDateTime::currentDateTime().toString(m_customTimestampFormat);
    }
    return QString();
}

// ──────────────────────────────────────────────
// 数据记录格式化
// ──────────────────────────────────────────────

QByteArray SerialDataLogger::formatRecord(const QByteArray& data,
                                          const QString& direction) const
{
    switch (m_format) {
    case LogFormat::Raw:
        return data;

    case LogFormat::Hex: {
        QByteArray line;
        if (m_timestampFormat != TimestampFormat::None) {
            line += "[" + formatTimestamp().toUtf8() + "] ";
        }
        line += direction.toUtf8() + " " +
                QByteArray::number(data.size()) + ": ";
        line += data.toHex(' ').toUpper() + "\n";
        return line;
    }

    case LogFormat::Ascii: {
        QByteArray line;
        if (m_timestampFormat != TimestampFormat::None) {
            line += "[" + formatTimestamp().toUtf8() + "] ";
        }
        line += direction.toUtf8() + ": " + data + "\n";
        return line;
    }

    case LogFormat::Csv: {
        QByteArray line;
        line += formatTimestamp().toUtf8() + ",";
        line += direction.toUtf8() + ",";
        line += QByteArray::number(data.size()) + ",";
        line += data.toHex() + ",";
        line += data.toPercentEncoding() + "\n";
        return line;
    }

    case LogFormat::Json: {
        QByteArray line;
        if (m_totalRecords > 0) {
            line += ",\n";
        }
        line += "{\"ts\":\"" + formatTimestamp().toUtf8() + "\",";
        line += "\"dir\":\"" + direction.toUtf8() + "\",";
        line += "\"len\":" + QByteArray::number(data.size()) + ",";
        line += "\"hex\":\"" + data.toHex() + "\",";
        line += "\"ascii\":" + QString::fromUtf8(data.toPercentEncoding()).toUtf8();
        line += "}";
        return line;
    }

    case LogFormat::Pcap:
        return buildPcapPacketHeader(static_cast<quint32>(data.size())) + data;
    }

    return data;
}

// ──────────────────────────────────────────────
// PCAP 头构建
// ──────────────────────────────────────────────

QByteArray SerialDataLogger::buildPcapGlobalHeader() const
{
    QByteArray header(24, 0);
    header[0] = static_cast<char>(kPcapMagic & 0xFF);
    header[1] = static_cast<char>((kPcapMagic >> 8) & 0xFF);
    header[2] = static_cast<char>((kPcapMagic >> 16) & 0xFF);
    header[3] = static_cast<char>((kPcapMagic >> 24) & 0xFF);
    header[4] = static_cast<char>(kPcapMajor & 0xFF);
    header[5] = static_cast<char>((kPcapMajor >> 8) & 0xFF);
    header[6] = static_cast<char>(kPcapMinor & 0xFF);
    header[7] = static_cast<char>((kPcapMinor >> 8) & 0xFF);
    // 字节 8-11: 时区偏移(GMT/UTC = 0)
    // 字节 12-15: 精度(0 = 微秒)
    // 字节 16-19: snaplen(65535)
    header[16] = static_cast<char>(0xFF);
    header[17] = static_cast<char>(0xFF);
    // 字节 20-23: 网络层类型(147 = LINKTYPE_USER0)
    header[20] = static_cast<char>(147);
    return header;
}

QByteArray SerialDataLogger::buildPcapPacketHeader(quint32 len) const
{
    QByteArray header(16, 0);
    const quint64 nowUs = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch()) * 1000ULL;
    const quint32 tsSec  = static_cast<quint32>(nowUs / 1000000ULL);
    const quint32 tsUsec = static_cast<quint32>(nowUs % 1000000ULL);
    // 小端序写入
    header[0]  = static_cast<char>(tsSec & 0xFF);
    header[1]  = static_cast<char>((tsSec >> 8) & 0xFF);
    header[2]  = static_cast<char>((tsSec >> 16) & 0xFF);
    header[3]  = static_cast<char>((tsSec >> 24) & 0xFF);
    header[4]  = static_cast<char>(tsUsec & 0xFF);
    header[5]  = static_cast<char>((tsUsec >> 8) & 0xFF);
    header[6]  = static_cast<char>((tsUsec >> 16) & 0xFF);
    header[7]  = static_cast<char>((tsUsec >> 24) & 0xFF);
    // 原始长度
    header[8]  = static_cast<char>(len & 0xFF);
    header[9]  = static_cast<char>((len >> 8) & 0xFF);
    header[10] = static_cast<char>((len >> 16) & 0xFF);
    header[11] = static_cast<char>((len >> 24) & 0xFF);
    // 实际长度(同原始)
    header[12] = header[8];
    header[13] = header[9];
    header[14] = header[10];
    header[15] = header[11];
    return header;
}

// ──────────────────────────────────────────────
// 文件写入
// ──────────────────────────────────────────────

void SerialDataLogger::writeToFile(const QByteArray& bytes)
{
    if (!m_file || bytes.isEmpty()) {
        return;
    }

    const qint64 written = m_file->write(bytes);
    if (written < 0) {
        emit error(tr("日志写入失败: %1").arg(m_file->errorString()));
        return;
    }

    m_currentFileSize += static_cast<quint64>(written);
}

// ──────────────────────────────────────────────
// 定时回调 — 见拆分文件
// ──────────────────────────────────────────────
