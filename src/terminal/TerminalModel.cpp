#include "TerminalModel.h"

TerminalModel::TerminalModel(QObject* parent)
    : QObject(parent)
{
    // 预分配环形缓冲区容量，避免运行时反复扩容
    m_buffer.resize(m_maxLines);
}

void TerminalModel::appendReceived(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);

    TerminalLine line;
    line.data = data;
    line.direction = DataDirection::Rx;
    line.timestamp = QDateTime::currentDateTime();
    m_rxBytes += data.size();

    appendLine(std::move(line));
}

void TerminalModel::appendSent(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);

    TerminalLine line;
    line.data = data;
    line.direction = DataDirection::Tx;
    line.timestamp = QDateTime::currentDateTime();
    m_txBytes += data.size();

    appendLine(std::move(line));
}

QVector<TerminalLine> TerminalModel::lines() const
{
    QMutexLocker locker(&m_mutex);

    QVector<TerminalLine> result;
    result.reserve(m_count);
    for (int i = 0; i < m_count; ++i) {
        result.append(m_buffer[physicalIndex(i)]);
    }
    return result;
}

QVector<TerminalLine> TerminalModel::lines(int start, int count) const
{
    QMutexLocker locker(&m_mutex);

    if (start < 0) start = 0;
    if (start >= m_count) return {};
    int actualCount = qMin(count, m_count - start);

    QVector<TerminalLine> result;
    result.reserve(actualCount);
    for (int i = 0; i < actualCount; ++i) {
        result.append(m_buffer[physicalIndex(start + i)]);
    }
    return result;
}

const TerminalLine& TerminalModel::lineAt(int index) const
{
    // 不加锁：只在主线程调用，与 appendLine 的写操作通过 Qt 信号槽机制串行化
    return m_buffer[physicalIndex(index)];
}

int TerminalModel::lineCount() const
{
    // 不加锁：原子读操作，且只在主线程调用
    return m_count;
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
    m_head = 0;
    m_count = 0;
    m_rxBytes = 0;
    m_txBytes = 0;
    emit dataCleared();
}

void TerminalModel::setMaxLines(int max)
{
    QMutexLocker locker(&m_mutex);

    if (max == m_maxLines) return;

    // 将旧数据按逻辑顺序拷贝到临时缓冲区
    QVector<TerminalLine> oldLines;
    oldLines.reserve(qMin(m_count, max));
    int copyStart = (m_count > max) ? (m_count - max) : 0;
    for (int i = copyStart; i < m_count; ++i) {
        oldLines.append(std::move(m_buffer[physicalIndex(i)]));
    }

    // 重新分配缓冲区
    m_maxLines = max;
    m_buffer.resize(max);

    // 将保留的数据放回新缓冲区
    int keepCount = qMin(static_cast<int>(oldLines.size()), max);
    for (int i = 0; i < keepCount; ++i) {
        m_buffer[i] = std::move(oldLines[i]);
    }

    m_head = 0;
    m_count = keepCount;
}

int TerminalModel::maxLines() const
{
    return m_maxLines;
}

int TerminalModel::physicalIndex(int logicalIndex) const
{
    return (m_head + logicalIndex) % m_buffer.size();
}

void TerminalModel::appendLine(TerminalLine&& line)
{
    // 必须在已持有 m_mutex 的情况下调用

    if (m_count < m_buffer.size()) {
        // 缓冲区未满，直接顺序写入
        m_buffer[m_count] = std::move(line);
        m_count++;
    } else {
        // 缓冲区已满，覆盖头指针位置的最旧数据
        m_buffer[m_head] = std::move(line);
        m_head = (m_head + 1) % m_buffer.size();
    }

    emit dataAppended(m_count - 1, 1);
}
