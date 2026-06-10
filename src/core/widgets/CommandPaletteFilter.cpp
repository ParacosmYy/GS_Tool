/**
 * @file CommandPaletteFilter.cpp
 * @brief 命令面板 - 搜索过滤与命令执行逻辑实现
 *
 * 从 CommandPalette.cpp 拆分而来，包含搜索变更处理、
 * 命令执行、列表刷新、模糊匹配算法和统计重置方法。
 */

#include "core/widgets/CommandPalette.h"

#include <QListWidgetItem>
#include <QRegularExpression>
#include <algorithm>

namespace {
constexpr int kNoMatch = 1000;

bool startsWithCaseInsensitive(const QString& text, const QString& prefix)
{
    return text.toLower().startsWith(prefix.toLower());
}

bool tokenStartsWithCaseInsensitive(const QString& text, const QString& prefix)
{
    const QString normalized = text.toLower();
    const QString needle = prefix.toLower();
    const auto tokens = normalized.split(QRegularExpression("[\\.\\-_\\s]+"), Qt::SkipEmptyParts);
    return std::any_of(tokens.cbegin(), tokens.cend(), [&needle](const QString& token) {
        return token.startsWith(needle);
    });
}
}

/** @brief 搜索框文字变更时触发，重新过滤并刷新命令列表 @param text 当前搜索文本 */
void CommandPalette::onSearchChanged(const QString& text)
{
    ++m_totalSearches;
    refreshList(text);
}

/** @brief 命令列表项激活时触发，执行对应命令并隐藏面板 @param item 激活的列表项指针 */
void CommandPalette::onItemActivated(QListWidgetItem* item)
{
    if (!item) return;
    if (!(item->flags() & Qt::ItemIsEnabled)) return;

    int idx = item->data(Qt::UserRole).toInt();
    if (idx >= 0 && idx < m_commands.size()) {
        const auto& cmd = m_commands[idx];
        if (cmd.action) {
            ++m_totalExecutions;
            cmd.action();
        }
        emit commandExecuted(cmd.id);
    }
    hidePalette();
}

/** @brief 根据过滤文本刷新命令列表，使用模糊子序列匹配 @param filter 过滤文本，为空时显示全部命令 */
void CommandPalette::refreshList(const QString& filter)
{
    m_listWidget->clear();
    m_filteredIndices.clear();

    QVector<QPair<int, int>> scoredIndices;
    for (int i = 0; i < m_commands.size(); ++i) {
        const auto& cmd = m_commands[i];
        const int score = matchScore(filter, cmd);
        if (score >= kNoMatch) continue;

        scoredIndices.append({score, i});
    }

    std::stable_sort(scoredIndices.begin(), scoredIndices.end(),
                     [](const auto& lhs, const auto& rhs) {
        return lhs.first < rhs.first;
    });

    for (const auto& scoredIndex : scoredIndices) {
        const int commandIndex = scoredIndex.second;
        const auto& cmd = m_commands[commandIndex];
        QString text = tr("%1  %2").arg(cmd.category, cmd.label);
        if (!cmd.shortcut.isEmpty())
            text += tr("  [%1]").arg(cmd.shortcut);

        auto* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, commandIndex);
        m_listWidget->addItem(item);
        m_filteredIndices.append(commandIndex);
    }

    if (m_listWidget->count() > 0)
        m_listWidget->setCurrentRow(0);
    else if (!filter.isEmpty()) {
        auto* emptyItem = new QListWidgetItem(tr("未找到匹配项"));
        emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsEnabled);
        m_listWidget->addItem(emptyItem);
    }
}

/** @brief 模糊子序列匹配算法，判断filter是否为target的子序列 @param filter 搜索过滤器文本 @param target 目标匹配文本 @return true表示匹配成功 */
bool CommandPalette::fuzzyMatch(const QString& filter, const QString& target) const
{
    if (filter.isEmpty()) return true;

    const QString f = filter.toLower();
    const QString t = target.toLower();

    int fi = 0;
    for (int ti = 0; ti < t.length() && fi < f.length(); ++ti) {
        if (t[ti] == f[fi])
            ++fi;
    }
    return fi == f.length();
}

/** @brief 计算命令匹配分数，前缀匹配优先于普通子序列匹配 @param filter 过滤文本 @param command 命令条目 @return 分数，kNoMatch表示不匹配 */
int CommandPalette::matchScore(const QString& filter, const CommandEntry& command) const
{
    if (filter.isEmpty()) return 0;

    if (startsWithCaseInsensitive(command.label, filter)) return 0;
    if (startsWithCaseInsensitive(command.id, filter)) return 1;
    if (tokenStartsWithCaseInsensitive(command.id, filter)) return 2;
    if (startsWithCaseInsensitive(command.category, filter)) return 2;
    if (startsWithCaseInsensitive(command.shortcut, filter)) return 3;
    if (fuzzyMatch(filter, command.label)) return 10;
    if (fuzzyMatch(filter, command.id)) return 11;
    if (fuzzyMatch(filter, command.category)) return 12;
    if (fuzzyMatch(filter, command.shortcut)) return 13;
    return kNoMatch;
}

/** @brief 重置命令面板统计计数器(搜索/执行/键盘事件) */
void CommandPalette::resetPaletteStatistics()
{
    m_totalSearches = 0;
    m_totalExecutions = 0;
    m_totalKeyEvents = 0;
}
