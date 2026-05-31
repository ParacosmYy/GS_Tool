/**
 * @file TerminalContextMenuManager.h
 * @brief Terminal context menu manager
 */
#ifndef TERMINALCONTEXTMENUMANAGER_H
#define TERMINALCONTEXTMENUMANAGER_H

#include <QObject>
#include <QMenu>

class QAction;
class QContextMenuEvent;

class TerminalContextMenuManager : public QObject {
    Q_OBJECT

signals:
    void copyRequested();
    void pasteRequested(const QString& text);
    void clearRequested();
    void selectAllRequested();
    void searchRequested();

public:
    explicit TerminalContextMenuManager(QObject* parent = nullptr);
    void showContextMenu(QContextMenuEvent* event, bool hasSelection);

private:
    QMenu* m_contextMenu;
    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_clearAction;
    QAction* m_selectAllAction;
    QAction* m_searchAction;
};

#endif // TERMINALCONTEXTMENUMANAGER_H
