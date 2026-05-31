#include "TerminalModel.h"

TerminalModel::TerminalModel(QObject* parent)
    : QObject(parent)
{
    // 预分配环形缓冲区容量，避免运行时反复扩容
    m_buffer.resize(m_maxLines);
}

void TerminalModel::appendReceived(const QByteArray& data)
{
    int newLineIndex;
    {
        QMutexLocker locker(&m_mutex);

        TerminalLine line;
        line.data = data;
        line.direction = DataDirection::Rx;
        line.timestamp = QDateTime::currentDateTime();
        m_rxBytes += data.size();

        appendLine(std::move(line));
        newLineIndex = m_count - 1;
    } // 锁已释放，安全发信号
    emit dataAppended(newLineIndex, 1);
}

void TerminalModel::appendSent(const QByteArray& data)
{
    int newLineIndex;
    {
        QMutexLocker locker(&m_mutex);

        TerminalLine line;
        line.data = data;
        line.direction = DataDirection::Tx;
        line.timestamp = QDateTime::currentDateTime();
        m_txBytes += data.size();

        appendLine(std::move(line));
        newLineIndex = m_count - 1;
    } // 锁已释放，安全发信号
    emit dataAppended(newLineIndex, 1);
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

/**
 * @brief 获取指定索引行的拷贝
 * @param index 行索引，范围 [0, lineCount())
 * @return 行数据拷贝；索引越界时返回空的 TerminalLine
 *
 * 运行时安全检查: 如果 index 越界，返回默认构造的 TerminalLine（空数据），
 * 并记录警告日志。Release 模式下 Q_ASSERT 会被编译器移除，
 * 因此使用实际的运行时检查替代断言。
 */
TerminalLine TerminalModel::lineAt(int index) const
{
    QMutexLocker locker(&m_mutex);

    // 运行时边界检查: 替代 Q_ASSERT，在 Release 模式下仍然有效
    if (index < 0 || index >= m_count) {
        qWarning() << "TerminalModel::lineAt: 索引越界，index=" << index
                   << "，有效范围 [0," << m_count << ")";
        return TerminalLine{};
    }

    return m_buffer[physicalIndex(index)];
}

int TerminalModel::lineCount() const
{
    QMutexLocker locker(&m_mutex);
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
    {
        QMutexLocker locker(&m_mutex);
        m_head = 0;
        m_count = 0;
        m_rxBytes = 0;
        m_txBytes = 0;
    } // 锁已释放，安全发信号
    emit dataCleared();
}

void TerminalModel::setMaxLines(int max)
{
    // 重入保护: 若 dataCleared 信号的槽函数回调 setMaxLines，
    // 直接返回，避免 m_buffer 的 resize 和 std::move 在两个调用栈中并发执行
    if (m_settingMaxLines) return;

    m_settingMaxLines = true;

    {
        QMutexLocker locker(&m_mutex);

        if (max == m_maxLines) {
            m_settingMaxLines = false;
            return;
        }

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
    } // 锁已释放，安全发信号

    // 通知视图数据已重置，避免显示过期内容
    emit dataCleared();

    m_settingMaxLines = false;
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
    // 注意: 不在此处 emit 信号，由调用者在释放锁后负责 emit，避免持锁发信号导致死锁

    if (m_count < m_buffer.size()) {
        // 缓冲区未满，直接顺序写入
        m_buffer[m_count] = std::move(line);
        m_count++;
    } else {
        // 缓冲区已满，覆盖头指针位置的最旧数据
        m_buffer[m_head] = std::move(line);
        m_head = (m_head + 1) % m_buffer.size();
    }
}
