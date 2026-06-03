/**
 * @file TerminalContextMenuManager.cpp
 * @brief 终端右键菜单管理器实现 - 菜单项创建、事件处理和使用统计
 *
 * 菜单项布局:
 *   复制(Ctrl+C) | 粘贴(Ctrl+V) | --- | 清屏 | 全选(Ctrl+A) | --- | 搜索(Ctrl+F)
 * 粘贴操作直接读取系统剪贴板内容，其他操作通过信号委托给外部。
 * 复制和搜索操作会递增对应的统计计数器。
 */
#include "terminal/menu/TerminalContextMenuManager.h"
#include <QAction>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QKeySequence>
#include <QApplication>

/** @brief 构造终端右键菜单管理器，创建复制/粘贴/清屏/全选/搜索菜单项 @param parent 父对象(通常为TerminalWidget) */
TerminalContextMenuManager::TerminalContextMenuManager(QObject* parent)
    : QObject(parent)
    , m_contextMenu(new QMenu(qobject_cast<QWidget*>(parent)))
    , m_copyAction(nullptr)
    , m_pasteAction(nullptr)
    , m_clearAction(nullptr)
    , m_selectAllAction(nullptr)
    , m_searchAction(nullptr)
{
    m_contextMenu->setObjectName("terminalContextMenu");

    m_copyAction = m_contextMenu->addAction(
        tr("复制") + QString("\t") + QKeySequence(QKeySequence::Copy).toString());
    connect(m_copyAction, &QAction::triggered, this, [this]() {
        ++m_totalCopyActions;
        emit copyRequested();
    });

    m_pasteAction = m_contextMenu->addAction(
        tr("粘贴") + QString("\t") + QKeySequence(QKeySequence::Paste).toString());
    connect(m_pasteAction, &QAction::triggered, this, [this]() {
        QString text = QApplication::clipboard()->text();
        if (!text.isEmpty()) emit pasteRequested(text);
    });

    m_contextMenu->addSeparator();

    m_clearAction = m_contextMenu->addAction(tr("清空"));
    connect(m_clearAction, &QAction::triggered, this, [this]() {
        emit clearRequested();
    });

    m_selectAllAction = m_contextMenu->addAction(
        tr("全选") + QString("\t") + QKeySequence(QKeySequence::SelectAll).toString());
    connect(m_selectAllAction, &QAction::triggered, this, [this]() {
        emit selectAllRequested();
    });

    m_contextMenu->addSeparator();

    m_searchAction = m_contextMenu->addAction(
        tr("搜索") + QString("\t") + QKeySequence(QKeySequence::Find).toString());
    connect(m_searchAction, &QAction::triggered, this, [this]() {
        ++m_totalSearchActions;
        emit searchRequested();
    });
}

/** @brief 显示右键菜单(根据是否有选中文本来启用/禁用复制按钮)，同时递增菜单弹出计数 @param event 右键菜单事件 @param hasSelection 当前是否有选中文本 */
void TerminalContextMenuManager::showContextMenu(QContextMenuEvent* event, bool hasSelection)
{
    if (!m_contextMenu) return;
    m_copyAction->setEnabled(hasSelection);
    ++m_totalMenuShows;
    m_contextMenu->popup(event->globalPos());
    event->accept();
}

// ── 统计计数器 Getter 实现 ──

/** @brief 获取菜单总弹出次数 @return 右键菜单累计显示次数 */
quint64 TerminalContextMenuManager::totalMenuShows() const
{
    return m_totalMenuShows;
}

/** @brief 获取复制操作总触发次数 @return 用户点击"复制"菜单项的累计次数 */
quint64 TerminalContextMenuManager::totalCopyActions() const
{
    return m_totalCopyActions;
}

/** @brief 获取搜索操作总触发次数 @return 用户点击"搜索"菜单项的累计次数 */
quint64 TerminalContextMenuManager::totalSearchActions() const
{
    return m_totalSearchActions;
}

/** @brief 重置所有统计计数器为零 */
void TerminalContextMenuManager::resetStats()
{
    m_totalMenuShows = 0;
    m_totalCopyActions = 0;
    m_totalSearchActions = 0;
}
