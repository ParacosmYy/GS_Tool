/**
 * @file TerminalContextMenuManager.cpp
 * @brief 终端右键菜单管理器实现 - 菜单项创建和事件处理
 *
 * 菜单项布局:
 *   复制(Ctrl+C) | 粘贴(Ctrl+V) | --- | 清屏 | 全选(Ctrl+A) | --- | 搜索(Ctrl+F)
 * 粘贴操作直接读取系统剪贴板内容，其他操作通过信号委托给外部。
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
        emit searchRequested();
    });
}

/** @brief 显示右键菜单(根据是否有选中文本来启用/禁用复制按钮) @param event 右键菜单事件 @param hasSelection 当前是否有选中文本 */
void TerminalContextMenuManager::showContextMenu(QContextMenuEvent* event, bool hasSelection)
{
    if (!m_contextMenu) return;
    m_copyAction->setEnabled(hasSelection);
    m_contextMenu->popup(event->globalPos());
    event->accept();
}
