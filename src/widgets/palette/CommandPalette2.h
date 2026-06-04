/**
 * @file CommandPalette2.h
 * @brief 命令面板组件 - 提供模糊搜索的命令快捷执行面板
 *
 * 职责:
 *   1. 管理命令注册表（名称、快捷键、回调动作）
 *   2. 提供模糊搜索过滤和键盘导航的弹出面板
 *   3. 回车执行选中命令，Esc关闭面板
 *
 * 设计参考: VS Code命令面板（Ctrl+P）
 */

#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QString>
#include <functional>
#include <QVBoxLayout>

/**
 * @brief 命令面板 - 模糊搜索命令快捷执行器
 *
 * 注册命令后通过 showPalette() 弹出面板，用户输入关键词过滤命令列表，
 * 回车执行选中命令。支持自定义命令名称、快捷键描述和回调函数。
 */
class CommandPalette : public QWidget {
    Q_OBJECT
public:
    /** @brief 命令动作回调类型 */
    using CommandAction = std::function<void()>;

    /**
     * @brief 构造命令面板
     * @param parent 父widget
     */
    explicit CommandPalette(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~CommandPalette() override;

    /**
     * @brief 注册一条命令
     * @param name 命令名称（同时用作搜索关键字）
     * @param shortcut 快捷键描述文本（如 "Ctrl+S"）
     * @param action 命令执行回调
     */
    void addCommand(const QString &name, const QString &shortcut, CommandAction action);

    /**
     * @brief 移除已注册的命令
     * @param name 命令名称
     */
    void removeCommand(const QString &name);

    /**
     * @brief 设置搜索过滤文本
     * @param text 过滤关键词
     */
    void setFilter(const QString &text);

    /** @brief 显示命令面板并聚焦搜索框 */
    void showPalette();

    /** @brief 隐藏命令面板 */
    void hidePalette();

    /**
     * @brief 获取已注册命令数量
     * @return 命令总数
     */
    int commandCount() const;

signals:
    /** @brief 命令执行完成 @param name 已执行的命令名称 */
    void commandExecuted(const QString &name);

    /** @brief 面板已显示 */
    void paletteShown();

    /** @brief 面板已隐藏 */
    void paletteHidden();

protected:
    /** @brief 键盘事件处理：Esc关闭面板 */
    void keyPressEvent(QKeyEvent *event) override;

private:
    /** @brief 回车键触发：执行当前选中的命令 */
    void onReturnPressed();

    /** @brief 搜索框文本变更：实时过滤命令列表 @param text 当前输入文本 */
    void onTextChanged(const QString &text);

    /** @brief 命令条目结构体 */
    struct CmdEntry {
        QString name;           ///< 命令名称
        QString shortcut;       ///< 快捷键描述
        CommandAction action;   ///< 执行回调
    };

    QMap<QString, CmdEntry> m_commands; ///< 命令注册表（按名称索引）
    QLineEdit *m_search = nullptr;      ///< 搜索输入框
    QListWidget *m_list = nullptr;      ///< 命令列表widget

    // ---- 统计计数器 ----
    quint64 m_totalCommandsExecuted = 0; ///< 总命令执行次数
    quint64 m_totalPaletteShows = 0;     ///< 总面板显示次数
    quint64 m_totalFilterChanges = 0;    ///< 总过滤变更次数

public:
    /** @brief 获取总命令执行次数 @return 累计执行次数 */
    quint64 totalCommandsExecuted() const { return m_totalCommandsExecuted; }
    /** @brief 获取总面板显示次数 @return 累计显示次数 */
    quint64 totalPaletteShows() const { return m_totalPaletteShows; }
    /** @brief 获取总过滤变更次数 @return 累计变更次数 */
    quint64 totalFilterChanges() const { return m_totalFilterChanges; }
    /** @brief 重置命令面板统计 */
    void resetPaletteStatistics() { m_totalCommandsExecuted = 0; m_totalPaletteShows = 0; m_totalFilterChanges = 0; }
};
