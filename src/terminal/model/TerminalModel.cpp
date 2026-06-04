/**
 * @file TerminalModel.cpp
 * @brief 终端数据模型实现 -- 管理终端显示数据的环形缓冲区
 *
 * 维护固定大小的环形缓冲区存储终端行数据，支持追加接收/发送数据、
 * 行数统计、方向过滤和内容搜索。线程安全设计。
 */
#include "terminal/model/TerminalModel.h"

/** @brief 构造函数，预分配环形缓冲区容量 @param parent 父对象 */
TerminalModel::TerminalModel(QObject* parent)
    : QObject(parent)
{
    // 预分配环形缓冲区容量，避免运行时反复扩容
    m_buffer.resize(m_maxLines);
}

/** @brief 追加接收数据到环形缓冲区 @param data 接收到的原始字节 */
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
        m_totalBytesReceived += static_cast<quint64>(data.size());  // 累计接收字节数统计

        appendLine(std::move(line));
        newLineIndex = m_count - 1;
    } // 锁已释放，安全发信号
    emit dataAppended(newLineIndex, 1);
}

/** @brief 追加发送数据到环形缓冲区 @param data 发送的原始字节 */
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
        m_totalBytesSent += static_cast<quint64>(data.size());  // 累计发送字节数统计

        appendLine(std::move(line));
        newLineIndex = m_count - 1;
    } // 锁已释放，安全发信号
    emit dataAppended(newLineIndex, 1);
}

// 行数据查询/字节统计getter已移至 TerminalModelQuery.cpp

/** @brief 清空所有缓冲区数据和统计计数，发射dataCleared信号 */
void TerminalModel::clear()
{
    {
        QMutexLocker locker(&m_mutex);
        ++m_totalClears;  ///< 统计: 数据清空次数递增(在锁内)
        m_head = 0;
        m_count = 0;
        m_rxBytes = 0;
        m_txBytes = 0;
    } // 锁已释放，安全发信号
    emit dataCleared();
}

/** @brief 设置最大行数并重新分配缓冲区，保留最新数据(含重入保护) @param max 新的最大行数 */
void TerminalModel::setMaxLines(int max)
{
    // 防御性校验: maxLines必须>=1，否则physicalIndex()中%m_buffer.size()会除零崩溃
    if (max < 1) max = 1;

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

/** @brief 返回最大行数 @return 最大行数 */
int TerminalModel::maxLines() const
{
    return m_maxLines;
}

/** @brief 将逻辑索引转换为环形缓冲区的物理索引 @param logicalIndex 逻辑索引 @return 物理索引 */
int TerminalModel::physicalIndex(int logicalIndex) const
{
    return (m_head + logicalIndex) % m_buffer.size();
}

/** @brief 内部追加一行到环形缓冲区(必须已持有m_mutex)，缓冲区满时覆盖最旧数据 @param line 行数据(右值引用，避免拷贝) */
void TerminalModel::appendLine(TerminalLine&& line)
{
    // 必须在已持有 m_mutex 的情况下调用
    // 注意: 不在此处 emit 信号，由调用者在释放锁后负责 emit，避免持锁发信号导致死锁

    ++m_totalLinesAdded;  // 每次追加行，累计行数统计

    // 跟踪最长行长度
    const quint64 lineLen = static_cast<quint64>(line.data.size());
    if (lineLen > m_maxLineLength) {
        m_maxLineLength = lineLen;
    }

    if (m_count < m_buffer.size()) {
        // 缓冲区未满，直接顺序写入
        m_buffer[m_count] = std::move(line);
        m_count++;
    } else {
        // 缓冲区已满，覆盖头指针位置的最旧数据
        ++m_totalMaxLinesReached;  ///< 统计: 环形缓冲区满覆盖递增
        m_buffer[m_head] = std::move(line);
        m_head = (m_head + 1) % m_buffer.size();
    }

    // 更新峰值行数统计
    if (static_cast<quint64>(m_count) > m_peakLineCount) {
        m_peakLineCount = static_cast<quint64>(m_count);
    }
}

// 统计getter/resetStats见 TerminalModelStats.cpp
