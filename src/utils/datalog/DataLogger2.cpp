#include "utils/datalog/DataLogger2.h"
#include <QDateTime>
#include <QTextStream>

DataLogger::DataLogger(QObject *parent) : QObject(parent) {}
DataLogger::~DataLogger() { close(); }

bool DataLogger::open(const QString &path) {
    m_file.setFileName(path);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) return false;
    emit fileOpened(path); return true;
}

void DataLogger::close() { if (m_file.isOpen()) { m_file.close(); emit fileClosed(); } }

void DataLogger::log(Level level, const QString &msg) {
    if (level < m_minLevel) return;
    static const char *tags[] = {"DEBUG","INFO","WARN","ERROR"};
    QString line = QString("[%1] [%2] %3").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).arg(tags[level]).arg(msg);
    writeEntry(line);
    emit logged(level, msg);
}

void DataLogger::logData(const QByteArray &data, const QString &tag) {
    QString line = QString("[%1] [DATA] %2 len=%3 hex=%4").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).arg(tag).arg(data.size()).arg(QString::fromLatin1(data.left(64).toHex()));
    writeEntry(line);
}

void DataLogger::setLevel(Level l) { m_minLevel = l; }
void DataLogger::setAutoFlush(bool e) { m_autoFlush = e; }
void DataLogger::setMaxFileSize(qint64 b) { m_maxSize = b; }
qint64 DataLogger::fileSize() const { return m_file.size(); }
bool DataLogger::isOpen() const { return m_file.isOpen(); }
int DataLogger::entryCount() const { return m_count; }

void DataLogger::writeEntry(const QString &line) {
    if (!m_file.isOpen()) return;
    if (m_file.size() >= m_maxSize) { emit fileFull(m_file.fileName()); return; }
    QTextStream out(&m_file); out << line << "\n";
    if (m_autoFlush) m_file.flush();
    m_count++;
}
