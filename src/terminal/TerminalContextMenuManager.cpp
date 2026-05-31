/**
 * @file TerminalContextMenuManager.cpp
 * @brief 终端右键菜单管理器实现 - 菜单项创建和事件处理
 *
 * 菜单项布局:
 *   复制(Ctrl+C) | 粘贴(Ctrl+V) | --- | 清屏 | 全选(Ctrl+A) | --- | 搜索(Ctrl+F)
 * 粘贴操作直接读取系统剪贴板内容，其他操作通过信号委托给外部。
 */
#include "terminal/TerminalContextMenuManager.h"
#include <QAction>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QKeySequence>
#include <QApplication>

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
        tr("Copy") + QString("\t") + QKeySequence(QKeySequence::Copy).toString());
    connect(m_copyAction, &QAction::triggered, this, [this]() {
        emit copyRequested();
    });

    m_pasteAction = m_contextMenu->addAction(
        tr("Paste") + QString("\t") + QKeySequence(QKeySequence::Paste).toString());
    connect(m_pasteAction, &QAction::triggered, this, [this]() {
        QString text = QApplication::clipboard()->text();
        if (!text.isEmpty()) emit pasteRequested(text);
    });

    m_contextMenu->addSeparator();

    m_clearAction = m_contextMenu->addAction(tr("Clear"));
    connect(m_clearAction, &QAction::triggered, this, [this]() {
        emit clearRequested();
    });

    m_selectAllAction = m_contextMenu->addAction(
        tr("Select All") + QString("\t") + QKeySequence(QKeySequence::SelectAll).toString());
    connect(m_selectAllAction, &QAction::triggered, this, [this]() {
        emit selectAllRequested();
    });

    m_contextMenu->addSeparator();

    m_searchAction = m_contextMenu->addAction(
        tr("Search") + QString("\t") + QKeySequence(QKeySequence::Find).toString());
    connect(m_searchAction, &QAction::triggered, this, [this]() {
        emit searchRequested();
    });
}

void TerminalContextMenuManager::showContextMenu(QContextMenuEvent* event, bool hasSelection)
{
    if (!m_contextMenu) return;
    m_copyAction->setEnabled(hasSelection);
    m_contextMenu->popup(event->globalPos());
    event->accept();
}
