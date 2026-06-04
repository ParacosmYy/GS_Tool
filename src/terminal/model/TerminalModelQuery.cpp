/**
 * @file TerminalModelQuery.cpp
 * @brief 终端数据模型 — 行数据查询与字节统计实现
 *
 * 从 TerminalModel.cpp 拆分而来，包含线程安全的行数据
 * 读取、行数查询和收发字节统计方法。
 */

#include "terminal/model/TerminalModel.h"

/** @brief 返回所有行的拷贝(线程安全) @return 行数据向量 */
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

/** @brief 返回指定范围的行拷贝(线程安全) @param start 起始行索引 @param count 请求数量 @return 行数据向量 */
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

/** @brief 获取指定索引行的拷贝，越界时返回空TerminalLine并记录警告 @param index 行索引，范围 [0, lineCount()) @return 行数据拷贝 */
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

/** @brief 返回当前行数(线程安全) @return 行数 */
int TerminalModel::lineCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_count;
}

/** @brief 返回接收总字节数 @return 字节数 */
quint64 TerminalModel::rxBytes() const
{
    QMutexLocker locker(&m_mutex);
    return m_rxBytes;
}

/** @brief 返回发送总字节数 @return 字节数 */
quint64 TerminalModel::txBytes() const
{
    QMutexLocker locker(&m_mutex);
    return m_txBytes;
}
