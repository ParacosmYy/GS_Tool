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

void SmartAutoComplete::setEntries(const QVector<AutoCompleteEntry>& entries)
{
    m_entries = entries;
}

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

void SmartAutoComplete::hideComplete()
{
    hide();
    m_listWidget->clear();
    m_currentPrefix.clear();
}

bool SmartAutoComplete::hasSelection() const
{
    return m_listWidget->currentRow() >= 0;
}

QString SmartAutoComplete::selectedText() const
{
    auto* item = m_listWidget->currentItem();
    return item ? item->text() : QString();
}

bool SmartAutoComplete::handleKeyEvent(QKeyEvent* event)
{
    if (!isVisible()) return false;

    switch (event->key()) {
    case Qt::Key_Down:
        if (m_listWidget->count() > 0) {
            int next = qMin(m_listWidget->currentRow() + 1,
                            m_listWidget->count() - 1);
            m_listWidget->setCurrentRow(next);
        }
        return true;

    case Qt::Key_Up:
        if (m_listWidget->count() > 0) {
            int prev = qMax(m_listWidget->currentRow() - 1, 0);
            m_listWidget->setCurrentRow(prev);
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
        hideComplete();
        return true;

    default:
        return false;
    }
}

// ──────────────────────── 私有方法 ────────────────────────

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

void SmartAutoComplete::resetAutoCompleteStatistics()
{
    m_totalSuggestions = 0;
    m_totalSelections = 0;
}
