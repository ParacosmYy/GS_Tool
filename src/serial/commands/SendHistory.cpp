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

    // 超过最大记录数时，从头部删除最旧的条目
    while (m_entries.size() > m_maxEntries) {
        m_entries.removeFirst();
    }

    emit historyChanged();
}

/** @brief 获取最近count条发送文本(从新到旧) @param count 请求数量 @return 文本列表 */
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

/** @brief 返回所有发送记录 @return 条目列表 */
QList<SendEntry> SendHistory::entries() const
{
    return m_entries;
}

/** @brief 按关键词搜索发送记录(大小写不敏感) @param keyword 搜索关键词 @return 匹配的条目列表 */
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

/** @brief 清空所有发送记录和统计计数器，发射historyChanged信号 */
void SendHistory::clear()
{
    m_entries.clear();
    m_freqMap.clear();
    m_totalSendCount = 0;
    m_totalRecords = 0;
    m_totalDuplicateSkips = 0;
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

/**
 * @brief 获取历史总发送次数
 * @return 总发送次数（含去重）
 */
int SendHistory::totalSendCount() const
{
    return m_totalSendCount;
}

/**
 * @brief 获取最常发送的命令
 * @param topN 返回前N条，默认10
 * @return 命令和频率的列表，按频率降序
 */
QList<QPair<QString, int>> SendHistory::mostFrequent(int topN) const
{
    /* 将频率表转为列表并排序 */
    QList<QPair<QString, int>> freqList;
    for (auto it = m_freqMap.constBegin(); it != m_freqMap.constEnd(); ++it) {
        freqList.append(qMakePair(it.key(), it.value()));
    }

    /* 按频率降序排序 */
    std::sort(freqList.begin(), freqList.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
        return a.second > b.second;
    });

    /* 取前 topN 条 */
    if (freqList.size() > topN) {
        freqList = freqList.mid(0, topN);
    }

    return freqList;
}

/**
 * @brief 获取发送统计摘要文本
 * @return 格式化的统计信息字符串
 */
QString SendHistory::statisticsSummary() const
{
    QString summary;
    summary += tr("总发送次数: %1\n").arg(m_totalSendCount);
    summary += tr("不同命令数: %1\n").arg(m_freqMap.size());
    summary += tr("历史记录数: %1/%2\n").arg(m_entries.size()).arg(m_maxEntries);

    if (!m_freqMap.isEmpty()) {
        summary += tr("\n最常用命令:\n");
        const auto top = mostFrequent(5);
        for (int i = 0; i < top.size(); ++i) {
            summary += QStringLiteral("  %1. %2 (%3次)\n")
                          .arg(i + 1)
                          .arg(top[i].first.left(30))
                          .arg(top[i].second);
        }
    }

    return summary.trimmed();
}
