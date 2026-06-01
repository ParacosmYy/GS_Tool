/**
 * @file SendHistory.cpp
 * @brief 发送历史管理器实现 — 记录和检索用户发送数据的历史
 *
 * 维护最近发送记录的环形缓冲区，支持去重、最大条目数限制，
 * 以及持久化到QSettings。
 */
#include "serial/SendHistory.h"

SendHistory::SendHistory(QObject* parent)
    : QObject(parent)
{
}

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
            return;
        }
    }

    // 构造新条目
    SendEntry entry;
    entry.text = text;
    entry.isHex = isHex;
    entry.time = QDateTime::currentDateTime();

    m_entries.append(entry);

    // 超过最大记录数时，从头部删除最旧的条目
    while (m_entries.size() > m_maxEntries) {
        m_entries.removeFirst();
    }

    emit historyChanged();
}

QStringList SendHistory::recentTexts(int count) const
{
    QStringList result;

    // 从最新的记录往前取，最多取count条
    int start = qMax(0, m_entries.size() - count);
    for (int i = m_entries.size() - 1; i >= start; --i) {
        result.append(m_entries[i].text);
    }

    return result;
}

QList<SendEntry> SendHistory::entries() const
{
    return m_entries;
}

QList<SendEntry> SendHistory::search(const QString& keyword) const
{
    QList<SendEntry> result;

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

void SendHistory::clear()
{
    m_entries.clear();
    emit historyChanged();
}

void SendHistory::setMaxEntries(int max)
{
    // 限制最大记录数的最小值为1，防止设为0导致异常
    m_maxEntries = qMax(1, max);

    // 如果当前记录数已经超过新的上限，裁剪掉多余的旧记录
    while (m_entries.size() > m_maxEntries) {
        m_entries.removeFirst();
    }
}
