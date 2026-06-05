/**
 * @file SendHistoryStats.cpp
 * @brief 发送历史管理器 - 统计查询与重置接口实现
 *
 * 从 SendHistory.cpp 拆分而来，包含频率分析、统计摘要、
 * 所有统计getter和resetStatistics方法。
 */

#include "serial/commands/SendHistory.h"

#include <algorithm>

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
            summary += tr("  %1. %2 (%3次)\n")
                          .arg(i + 1)
                          .arg(top[i].first.left(30))
                          .arg(top[i].second);
        }
    }

    return summary.trimmed();
}

/** @brief 获取历史总发送次数 @return 总发送次数（含去重） */
quint64 SendHistory::totalSendCount() const
{
    return m_totalSendCount;
}

/** @brief 获取历史记录总条目数（累计添加，含去重跳过） @return 累计添加的记录总数 */
quint64 SendHistory::totalRecords() const
{
    return m_totalRecords;
}

/** @brief 获取因连续重复而被跳过的去重次数 @return 去重跳过次数 */
quint64 SendHistory::totalDuplicateSkips() const
{
    return m_totalDuplicateSkips;
}

/** @brief 获取搜索调用总次数 @return 累计搜索次数 */
quint64 SendHistory::totalSearches() const
{
    return m_totalSearches;
}

/** @brief 获取清空操作总次数 @return 累计清空次数 */
quint64 SendHistory::totalClears() const
{
    return m_totalClears;
}

/** @brief 获取历史记录访问总次数（recentTexts/entries/search调用合计） @return 累计访问次数 */
quint64 SendHistory::totalHistoryAccesses() const
{
    return m_totalHistoryAccesses;
}

/**
 * @brief 重置所有统计计数器
 *
 * 将 totalRecords、totalDuplicateSkips、totalSendCount、totalHistoryAccesses 全部归零，
 * 同时清空频率表和历史列表。
 */
void SendHistory::resetStatistics()
{
    m_totalRecords = 0;
    m_totalDuplicateSkips = 0;
    m_totalSendCount = 0;
    m_totalSearches = 0;
    m_totalClears = 0;
    m_totalHistoryAccesses = 0;
    m_freqMap.clear();
    m_entries.clear();
    emit historyChanged();
}
