/**
 * @file TerminalContextMenuManager.h
 * @brief 终端右键菜单管理器 - 管理终端控件的上下文菜单
 *
 * 提供复制、粘贴、清屏、全选、搜索等右键菜单操作。
 * 菜单项通过信号通知外部执行实际操作，保持管理器的无状态性。
 *
 * 协作关系:
 *   - TerminalWidget: 持有本类实例，在 contextMenuEvent 中委托调用
 *   - QContextMenuEvent: 提供右键点击位置
 */

#ifndef TERMINALCONTEXTMENUMANAGER_H
#define TERMINALCONTEXTMENUMANAGER_H

#include <QObject>
#include <QMenu>

class QAction;
class QContextMenuEvent;

/**
 * @brief 终端右键菜单管理器 - 提供标准上下文菜单操作
 *
 * 菜单项包含快捷键提示(Ctrl+C/V/A/Ctrl+F)，根据选中状态
 * 动态启用/禁用复制按钮。通过信号通知外部执行实际业务。
 */
class TerminalContextMenuManager : public QObject {
    Q_OBJECT

signals:
    /**
     * @brief 用户点击"复制"菜单项时发射
     * @sa TerminalWidget 执行实际的文本复制到剪贴板
     */
    void copyRequested();

    /**
     * @brief 用户点击"粘贴"菜单项时发射
     * @param text 从系统剪贴板读取的文本内容
     * @sa TerminalWidget 执行实际的文本发送到连接
     */
    void pasteRequested(const QString& text);

    /**
     * @brief 用户点击"清屏"菜单项时发射
     * @sa TerminalWidget 执行实际的终端内容清除
     */
    void clearRequested();

    /**
     * @brief 用户点击"全选"菜单项时发射
     * @sa TerminalWidget 执行实际的文本全选操作
     */
    void selectAllRequested();

    /**
     * @brief 用户点击"搜索"菜单项时发射
     * @sa TerminalWidget 打开/聚焦搜索栏
     */
    void searchRequested();

public:
    /**
     * @brief 构造终端右键菜单管理器
     * @param parent 父对象，通常为 TerminalWidget 实例
     */
    explicit TerminalContextMenuManager(QObject* parent = nullptr);

    /**
     * @brief 显示右键上下文菜单
     * @param event 上下文菜单事件，提供全局弹出位置
     * @param hasSelection 终端中是否有选中的文本，控制复制按钮的启用状态
     */
    void showContextMenu(QContextMenuEvent* event, bool hasSelection);

private:
    QMenu* m_contextMenu;           ///< 右键菜单实例，包含所有菜单项
    QAction* m_copyAction;          ///< "复制"菜单项(Ctrl+C)
    QAction* m_pasteAction;         ///< "粘贴"菜单项(Ctrl+V)
    QAction* m_clearAction;         ///< "清屏"菜单项(无快捷键)
    QAction* m_selectAllAction;     ///< "全选"菜单项(Ctrl+A)
    QAction* m_searchAction;        ///< "搜索"菜单项(Ctrl+F)
};

#endif // TERMINALCONTEXTMENUMANAGER_H
