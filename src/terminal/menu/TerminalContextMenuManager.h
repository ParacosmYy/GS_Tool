/**
 * @file TerminalContextMenuManager.h
 * @brief 终端右键菜单管理器 - 管理终端控件的上下文菜单及菜单操作统计
 *
 * 提供复制、粘贴、清屏、全选、搜索等右键菜单操作。
 * 菜单项通过信号通知外部执行实际操作，保持管理器的无状态性。
 * 同时维护菜单操作统计计数器(菜单显示次数、复制/搜索操作次数)。
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
 * @brief 终端右键菜单管理器 - 提供标准上下文菜单操作及使用统计
 *
 * 菜单项包含快捷键提示(Ctrl+C/V/A/Ctrl+F)，根据选中状态
 * 动态启用/禁用复制按钮。通过信号通知外部执行实际业务。
 * 统计计数器跟踪菜单弹出次数和各项操作的触发频率。
 */
class TerminalContextMenuManager : public QObject {
    Q_OBJECT

signals:
    /** @brief 用户点击"复制"菜单项时发射 */
    void copyRequested();

    /** @brief 用户点击"粘贴"菜单项时发射 @param text 从系统剪贴板读取的文本内容 */
    void pasteRequested(const QString& text);

    /** @brief 用户点击"清屏"菜单项时发射 */
    void clearRequested();

    /** @brief 用户点击"全选"菜单项时发射 */
    void selectAllRequested();

    /** @brief 用户点击"搜索"菜单项时发射 */
    void searchRequested();

public:
    /** @brief 构造终端右键菜单管理器 @param parent 父对象，通常为 TerminalWidget 实例 */
    explicit TerminalContextMenuManager(QObject* parent = nullptr);

    /** @brief 显示右键上下文菜单 @param event 上下文菜单事件 @param hasSelection 是否有选中文本 */
    void showContextMenu(QContextMenuEvent* event, bool hasSelection);

    // ── 统计计数器 Getter ──

    /** @brief 获取菜单总弹出次数 @return 右键菜单累计显示次数 */
    quint64 totalMenuShows() const;

    /** @brief 获取复制操作总触发次数 @return 用户点击"复制"菜单项的累计次数 */
    quint64 totalCopyActions() const;

    /** @brief 获取粘贴操作总触发次数 @return 用户点击"粘贴"菜单项的累计次数 */
    quint64 totalPasteActions() const;

    /** @brief 获取清屏操作总触发次数 @return 用户点击"清屏"菜单项的累计次数 */
    quint64 totalClearActions() const;

    /** @brief 获取搜索操作总触发次数 @return 用户点击"搜索"菜单项的累计次数 */
    quint64 totalSearchActions() const;

    /**
     * @brief 获取菜单项总触发次数
     * @return 所有菜单项(复制/粘贴/清屏/全选/搜索)被点击的累计次数
     */
    quint64 totalActionsTriggered() const;

    /** @brief 重置所有统计计数器为零 */
    void resetStats();

private:
    QMenu* m_contextMenu;           ///< 右键菜单实例，包含所有菜单项
    QAction* m_copyAction;          ///< "复制"菜单项(Ctrl+C)
    QAction* m_pasteAction;         ///< "粘贴"菜单项(Ctrl+V)
    QAction* m_clearAction;         ///< "清屏"菜单项(无快捷键)
    QAction* m_selectAllAction;     ///< "全选"菜单项(Ctrl+A)
    QAction* m_searchAction;        ///< "搜索"菜单项(Ctrl+F)

    // ── 统计计数器 ──
    quint64 m_totalMenuShows = 0;       ///< 菜单总弹出次数
    quint64 m_totalCopyActions = 0;     ///< 复制操作总触发次数
    quint64 m_totalPasteActions = 0;    ///< 粘贴操作总触发次数
    quint64 m_totalClearActions = 0;    ///< 清屏操作总触发次数
    quint64 m_totalSearchActions = 0;   ///< 搜索操作总触发次数
    quint64 m_totalActionsTriggered = 0; ///< 菜单项总触发次数(含所有操作)
};

#endif // TERMINALCONTEXTMENUMANAGER_H
