/**
 * @file SendHistory.cpp
 * @brief 发送历史管理器实现 — 记录和检索用户发送数据的历史
 *
 * 维护最近发送记录的环形缓冲区，支持去重、最大条目数限制，
 * 以及持久化到QSettings。
 */
#include "serial/commands/SendHistory.h"

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SendHistory::SendHistory(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 添加一条发送记录
 *
 * 空内容忽略，与最后一条相同时去重(避免连续发送同一命令导致历史刷屏)。
 * 超过最大记录数时自动淘汰最旧条目。
 * @param text 发送文本内容
 * @param isHex 是否为HEX模式发送
 */
void SendHistory::addEntry(const QString& text, bool isHex)
{
    // 忽略空内容
    if (text.isEmpty()) {
        return;
    }

    // 与最后一条内容相同时不添加，避免连续发送同一命令导致历史刷屏
    if (!m_entries.isEmpty()) {
        const SendEntry& last = m_entries.last();
        if (last.text == text && last.isHex == isHex) {
            ++m_totalDuplicateSkips;
            return;
        }
    }

    // 构造新条目
    SendEntry entry;
    entry.text = text;
    entry.isHex = isHex;
    entry.time = QDateTime::currentDateTime();

    m_entries.append(entry);

    // 更新频率统计和累计记录数
    m_freqMap[text]++;
    m_totalSendCount++;
    ++m_totalRecords;

    // 超过最大记录数时，从头部删除最旧的条目并同步频率映射
    while (m_entries.size() > m_maxEntries) {
        const QString oldText = m_entries.first().text;
        if (m_freqMap.contains(oldText)) {
            if (--m_freqMap[oldText] <= 0)
                m_freqMap.remove(oldText);
        }
        m_entries.removeFirst();
    }

    emit historyChanged();
}

/** @brief 获取最近count条发送文本(从新到旧) @param count 请求数量 @return 文本列表 */
QStringList SendHistory::recentTexts(int count) const
{
    ++m_totalHistoryAccesses;
    QStringList result;

    // 从最新的记录往前取，最多取count条
    int start = qMax(0, m_entries.size() - count);
    for (int i = m_entries.size() - 1; i >= start; --i) {
        result.append(m_entries[i].text);
    }

    return result;
}

/** @brief 返回所有发送记录 @return 条目列表 */
QList<SendEntry> SendHistory::entries() const
{
    ++m_totalHistoryAccesses;
    return m_entries;
}

/** @brief 按关键词搜索发送记录(大小写不敏感) @param keyword 搜索关键词 @return 匹配的条目列表 */
QList<SendEntry> SendHistory::search(const QString& keyword) const
{
    QList<SendEntry> result;
    ++m_totalSearches;
    ++m_totalHistoryAccesses;

    if (keyword.isEmpty()) {
        // 关键词为空时返回所有记录
        return m_entries;
    }

    // 遍历所有条目，大小写不敏感匹配
    for (const SendEntry& entry : m_entries) {
        if (entry.text.contains(keyword, Qt::CaseInsensitive)) {
            result.append(entry);
        }
    }

    return result;
}

/** @brief 清空所有发送记录和统计计数器，发射historyChanged信号 */
void SendHistory::clear()
{
    m_entries.clear();
    m_freqMap.clear();
    m_totalSendCount = 0;
    m_totalRecords = 0;
    m_totalDuplicateSkips = 0;
    ++m_totalClears;
    emit historyChanged();
}

/**
 * @brief 设置最大记录数
 *
 * 最小值为1(防止设为0)。如果当前记录数已超过新上限，裁剪掉多余的旧记录。
 * @param max 新的最大记录数
 */
void SendHistory::setMaxEntries(int max)
{
    // 限制最大记录数的最小值为1，防止设为0导致异常
    m_maxEntries = qMax(1, max);

    // 如果当前记录数已经超过新的上限，裁剪掉多余的旧记录
    while (m_entries.size() > m_maxEntries) {
        m_entries.removeFirst();
    }
}

// 频率分析/统计getter/resetStatistics见 SendHistoryStats.cpp
