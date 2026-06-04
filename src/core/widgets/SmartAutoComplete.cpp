/**
 * @file SmartAutoComplete.cpp
 * @brief 智能自动补全实现 — 前缀匹配 + 频率排序弹出列表
 */

#include "core/widgets/SmartAutoComplete.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <algorithm>

// ──────────────────────── 构造 ────────────────────────

/** @brief 构造智能自动补全弹出列表控件 @param parent 父控件指针 */
SmartAutoComplete::SmartAutoComplete(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setObjectName("smartAutoComplete");
    setAttribute(Qt::WA_TranslucentBackground, false);
    setFixedWidth(400);
    setMaximumHeight(200);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_listWidget = new QListWidget(this);
    m_listWidget->setObjectName("autoCompleteList");
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(m_listWidget);

    connect(m_listWidget, &QListWidget::itemClicked,
            this, [this](QListWidgetItem* item) {
        if (item) {
            ++m_totalSelections;
            emit entrySelected(item->text());
        }
        hideComplete();
    });
}

// ──────────────────────── 公开接口 ────────────────────────

/** @brief 设置补全候选词条目列表 @param entries 包含文字/频率/最后使用时间的候选条目集合 */
void SmartAutoComplete::setEntries(const QVector<AutoCompleteEntry>& entries)
{
    m_entries = entries;
}

/** @brief 根据前缀过滤并显示补全弹出列表 @param prefix 当前输入前缀 @param position 弹出列表的屏幕坐标位置 */
void SmartAutoComplete::showForPrefix(const QString& prefix, const QPoint& position)
{
    m_currentPrefix = prefix;
    m_listWidget->clear();

    auto indices = filterEntries(prefix);
    sortEntries(indices);

    for (int idx : indices) {
        m_listWidget->addItem(m_entries[idx].text);
    }

    if (m_listWidget->count() == 0) {
        ++m_totalNoMatchHides;
        hide();
        return;
    }

    // 限制可见行数，每行 28px + 4px 边距
    const int visibleCount = qMin(m_listWidget->count(), 8);
    setFixedHeight(visibleCount * 28 + 4);

    move(position);
    show();
    ++m_totalSuggestions;

    if (m_listWidget->count() > 0) {
        m_listWidget->setCurrentRow(0);
    }
}

/** @brief 隐藏补全弹出列表并清空当前前缀状态 */
void SmartAutoComplete::hideComplete()
{
    hide();
    m_listWidget->clear();
    m_currentPrefix.clear();
}

/** @brief 查询列表中是否有选中项 @return true表示当前有选中行 */
bool SmartAutoComplete::hasSelection() const
{
    return m_listWidget->currentRow() >= 0;
}

/** @brief 获取当前选中项的文字内容 @return 选中项文字，无选中时返回空字符串 */
QString SmartAutoComplete::selectedText() const
{
    auto* item = m_listWidget->currentItem();
    return item ? item->text() : QString();
}

/** @brief 处理键盘事件(上下导航/回车选择/ESC关闭) @param event 键盘事件指针 @return true表示事件已处理，false表示未处理 */
bool SmartAutoComplete::handleKeyEvent(QKeyEvent* event)
{
    if (!isVisible()) return false;

    switch (event->key()) {
    case Qt::Key_Down:
        if (m_listWidget->count() > 0) {
            int next = qMin(m_listWidget->currentRow() + 1,
                            m_listWidget->count() - 1);
            m_listWidget->setCurrentRow(next);
            ++m_totalKeyNavigations;
        }
        return true;

    case Qt::Key_Up:
        if (m_listWidget->count() > 0) {
            int prev = qMax(m_listWidget->currentRow() - 1, 0);
            m_listWidget->setCurrentRow(prev);
            ++m_totalKeyNavigations;
        }
        return true;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (hasSelection()) {
            ++m_totalSelections;
            emit entrySelected(selectedText());
            hideComplete();
        }
        return true;

    case Qt::Key_Escape:
        ++m_totalCancellations;
        hideComplete();
        return true;

    default:
        return false;
    }
}

// ──────────────────────── 私有方法 ────────────────────────

/** @brief 根据前缀过滤候选条目，返回匹配的索引列表 @param prefix 过滤前缀，为空时返回全部 @return 匹配条目的索引向量 */
QVector<int> SmartAutoComplete::filterEntries(const QString& prefix) const
{
    QVector<int> result;
    if (prefix.isEmpty()) {
        // 空前缀返回全部索引
        result.reserve(m_entries.size());
        for (int i = 0; i < m_entries.size(); ++i)
            result.append(i);
        return result;
    }

    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].text.startsWith(prefix, Qt::CaseInsensitive)) {
            result.append(i);
        }
    }
    return result;
}

/** @brief 按频率降序和时间降序对索引列表进行排序 @param indices 待排序的索引向量引用 */
void SmartAutoComplete::sortEntries(QVector<int>& indices) const
{
    std::sort(indices.begin(), indices.end(), [this](int a, int b) {
        // 频率降序
        if (m_entries[a].frequency != m_entries[b].frequency)
            return m_entries[a].frequency > m_entries[b].frequency;
        // 频率相同 → 时间降序
        return m_entries[a].lastUsed > m_entries[b].lastUsed;
    });
}

// ──────────────────────── 统计重置 ────────────────────────

/** @brief 重置自动补全统计计数器(建议次数/选择次数/取消/导航/无匹配隐藏) */
void SmartAutoComplete::resetAutoCompleteStatistics()
{
    m_totalSuggestions = 0;
    m_totalSelections = 0;
    m_totalCancellations = 0;
    m_totalKeyNavigations = 0;
    m_totalNoMatchHides = 0;
}
