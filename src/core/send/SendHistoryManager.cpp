/**
 * @file SendHistoryManager.cpp
 * @brief 发送历史管理器实现 - 自动补全、频率聚合、键盘导航
 */

#include "core/send/SendHistoryManager.h"
#include "serial/commands/SendHistory.h"
#include "core/widgets/SmartAutoComplete.h"

#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QKeyEvent>
#include <QMap>

namespace {
/** @brief 从发送历史聚合构建补全条目（按文本去重，累加频率，保留最近时间戳） */
QVector<AutoCompleteEntry> buildAggregatedEntries(SendHistory* history)
{
    QMap<QString, AutoCompleteEntry> agg;
    for (const auto& e : history->entries()) {
        auto& item = agg[e.text];
        item.text = e.text;
        item.frequency++;
        const qint64 ts = e.time.toMSecsSinceEpoch();
        if (ts > item.lastUsed) item.lastUsed = ts;
    }
    return agg.values().toVector();
}
} // anonymous namespace

SendHistoryManager::SendHistoryManager(SendHistory* history, QObject* parent)
    : QObject(parent)
    , m_sendHistory(history)
{
}

void SendHistoryManager::setupAutoComplete(QLineEdit* input, QWidget* parentWidget)
{
    m_input = input;

    // 基本补全器（QCompleter 内建前缀匹配，作为备选保留）
    m_completerModel = new QStringListModel(m_sendHistory->recentTexts(), this);
    m_completer = new QCompleter(m_completerModel, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_input->setCompleter(m_completer);

    // 智能补全弹出列表（频率排序前缀匹配）
    m_smartComplete = new SmartAutoComplete(parentWidget);
    m_smartComplete->setObjectName("smartAutoComplete");
    m_smartComplete->setEntries(buildAggregatedEntries(m_sendHistory));

    // 安装事件过滤器拦截键盘导航（Up/Down/Enter/Escape）
    m_input->installEventFilter(this);

    // 历史变更 → 刷新两个补全数据源
    connect(m_sendHistory, &SendHistory::historyChanged, this, [this]() {
        refreshCompletions();
    });

    // 输入变化 → 触发智能补全弹出
    connect(m_input, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (text.isEmpty()) {
            m_smartComplete->hideComplete();
            return;
        }
        const QPoint pos = m_input->mapToGlobal(QPoint(0, m_input->height()));
        m_smartComplete->showForPrefix(text, pos);
    });

    // 用户选择补全项 → 填入输入框
    connect(m_smartComplete, &SmartAutoComplete::entrySelected, this, [this](const QString& text) {
        m_smartComplete->hideComplete();
        m_input->blockSignals(true);
        m_input->setText(text);
        m_input->blockSignals(false);
        ++m_totalRecalls;  // 累计召回(补全选中)计数
    });
}

void SendHistoryManager::recordHistory(const QString& text, bool isHex)
{
    m_sendHistory->addEntry(text, isHex);
    ++m_totalAdds;  // 累计添加计数
}

SmartAutoComplete* SendHistoryManager::smartComplete() const
{
    return m_smartComplete;
}

bool SendHistoryManager::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_input && event->type() == QEvent::KeyPress && m_smartComplete) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (m_smartComplete->isVisible()) {
                m_smartComplete->handleKeyEvent(keyEvent);
                return true;
            }
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (m_smartComplete->hasSelection()) {
                m_smartComplete->hideComplete();
                m_input->blockSignals(true);
                m_input->setText(m_smartComplete->selectedText());
                m_input->blockSignals(false);
                ++m_totalRecalls;  // 累计召回(键盘选中补全)计数
                return true;
            }
            break;
        case Qt::Key_Escape:
            if (m_smartComplete->isVisible()) {
                m_smartComplete->hideComplete();
                return true;
            }
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(watched, event);
}

void SendHistoryManager::refreshCompletions()
{
    if (m_completerModel) {
        m_completerModel->setStringList(m_sendHistory->recentTexts());
    }
    if (m_smartComplete) {
        m_smartComplete->setEntries(buildAggregatedEntries(m_sendHistory));
    }
}

// ---- 统计计数器接口 ----

/** @brief 获取累计添加的历史记录数 @return 添加次数 */
quint64 SendHistoryManager::totalAdds() const
{
    return m_totalAdds;
}

/** @brief 获取累计清空历史的次数 @return 清空次数 */
quint64 SendHistoryManager::totalClears() const
{
    return m_totalClears;
}

/** @brief 获取累计召回(补全选中)的次数 @return 召回次数 */
quint64 SendHistoryManager::totalRecalls() const
{
    return m_totalRecalls;
}

/** @brief 重置所有统计计数器(添加/清空/召回) */
void SendHistoryManager::resetHistoryStatistics()
{
    m_totalAdds = 0;
    m_totalClears = 0;
    m_totalRecalls = 0;
}
