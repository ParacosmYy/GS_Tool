#include "TerminalModel.h"

TerminalModel::TerminalModel(QObject* parent)
    : QObject(parent)
{
}

void TerminalModel::appendReceived(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);

    TerminalLine line;
    line.data = data;
    line.direction = DataDirection::Rx;
    line.timestamp = QDateTime::currentDateTime();
    m_lines.append(line);
    m_rxBytes += data.size();

    // 超过最大行数时，删除最旧的行
    while (m_lines.size() > m_maxLines) {
        m_lines.removeFirst();
    }

    emit dataAppended(m_lines.size() - 1, 1);
}

void TerminalModel::appendSent(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);

    TerminalLine line;
    line.data = data;
    line.direction = DataDirection::Tx;
    line.timestamp = QDateTime::currentDateTime();
    m_lines.append(line);
    m_txBytes += data.size();

    while (m_lines.size() > m_maxLines) {
        m_lines.removeFirst();
    }

    emit dataAppended(m_lines.size() - 1, 1);
}

QVector<TerminalLine> TerminalModel::lines() const
{
    QMutexLocker locker(&m_mutex);
    return m_lines;
}

int TerminalModel::lineCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_lines.size();
}

quint64 TerminalModel::rxBytes() const
{
    QMutexLocker locker(&m_mutex);
    return m_rxBytes;
}

quint64 TerminalModel::txBytes() const
{
    QMutexLocker locker(&m_mutex);
    return m_txBytes;
}

void TerminalModel::clear()
{
    QMutexLocker locker(&m_mutex);
    m_lines.clear();
    m_rxBytes = 0;
    m_txBytes = 0;
    emit dataCleared();
}

void TerminalModel::setMaxLines(int max)
{
    m_maxLines = max;
}
