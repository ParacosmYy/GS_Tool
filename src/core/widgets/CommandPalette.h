/**
 * @file CommandPalette.h
 * @brief 命令面板 — Ctrl+P 触发的模糊搜索面板快速导航
 *
 * 浮动在主窗口中央的搜索面板，列出所有可导航面板和操作。
 * 用户输入关键词进行模糊过滤，回车执行选中项。
 *
 * 设计模式: 命令(Command) + 观察者(Observer)
 * 协作: NavigationController(切换面板) / PanelManager(面板列表)
 */
#ifndef COMMAND_PALETTE_H
#define COMMAND_PALETTE_H

#include <QWidget>
#include <functional>
#include <QVector>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QVBoxLayout;

/** @brief 命令面板条目 */
struct CommandEntry {
    QString id;                     ///< 命令标识(如 "nav.terminal")
    QString category;               ///< 分类(如 "导航", "操作")
    QString label;                  ///< 显示标签
    QString shortcut;               ///< 快捷键提示(可选)
    std::function<void()> action;   ///< 执行回调
};

/**
 * @brief 命令面板 — Ctrl+P 模糊搜索快速导航
 *
 * 使用场景: 快速切换面板、执行常用操作、键盘驱动工作流
 */
class CommandPalette : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造命令面板 @param parent 父窗口(通常是MainWindow) */
    explicit CommandPalette(QWidget* parent = nullptr);

    /** @brief 注册命令条目 @param entry 命令条目 */
    void registerCommand(const CommandEntry& entry);
    /** @brief 批量注册命令 @param entries 命令列表 */
    void registerCommands(const QVector<CommandEntry>& entries);

    /** @brief 显示命令面板(清空输入、刷新列表、设置焦点) */
    void showPalette();
    /** @brief 隐藏命令面板 */
    void hidePalette();

signals:
    /** @brief 命令执行信号 */
    void commandExecuted(const QString& id);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;  ///< ESC关闭/点击外部关闭
    void paintEvent(QPaintEvent* event) override;            ///< 绘制半透明背景遮罩

private slots:
    void onSearchChanged(const QString& text);     ///< 搜索框文本变化时过滤列表
    void onItemActivated(QListWidgetItem* item);   ///< 列表项激活时执行命令

private:
    void refreshList(const QString& filter = QString());  ///< 刷新过滤后的命令列表
    bool fuzzyMatch(const QString& filter, const QString& target) const; ///< 模糊子序列匹配

    QWidget* m_panelWidget = nullptr;       ///< objectName="commandPalettePanel"
    QLineEdit* m_searchEdit = nullptr;      ///< objectName="commandPaletteSearch"
    QListWidget* m_listWidget = nullptr;    ///< objectName="commandPaletteList"

    QVector<CommandEntry> m_commands;       ///< 全部注册命令
    QVector<int> m_filteredIndices;         ///< 当前过滤后的索引
};

#endif // COMMAND_PALETTE_H
