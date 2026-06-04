/**
 * @file ToolbarController.h
 * @brief 工具栏控制器 - 管理顶部工具栏的按钮布局和交互逻辑
 */

#ifndef TOOLBARCONTROLLER_H
#define TOOLBARCONTROLLER_H

#include <QObject>
#include <QStringList>

class QMainWindow;
class QToolBar;
class QComboBox;
class QAction;
class RecordingController;

/** @brief 工具栏控制器 - 管理主工具栏所有控件的创建和事件转发。职责: 创建控件/设置objectName/信号转发/主题语言管理。设计模式: 外观模式+观察者模式 */
class ToolbarController : public QObject {
    Q_OBJECT

public:
    /** @brief 构造工具栏控制器 @param recordingController 录制控制器实例 @param parent 父对象 */
    explicit ToolbarController(RecordingController* recordingController, QObject* parent = nullptr);
    /** @brief 创建并返回工具栏(显示模式|时间戳|清屏|导出|背景|录制|终端布局|主题|语言) @param parent 主窗口父控件 @return 工具栏指针 */
    QToolBar* createToolbar(QMainWindow* parent);
    /** @brief 创建显示模式控件组(显示模式/终端布局/时间戳/方向前缀/清屏) @param toolbar 目标工具栏 */
    void createDisplayModeGroup(QToolBar* toolbar);
    /** @brief 创建连接控制组(导出/背景/录制回放/主题/语言) @param toolbar 目标工具栏 */
    void createConnectionGroup(QToolBar* toolbar);
    /** @brief 获取工具栏指针 @return 工具栏实例，未创建返回nullptr */
    QToolBar* toolbar() const;
    /** @brief 设置可用主题列表 @param themes 主题名称列表 */
    void setAvailableThemes(const QStringList& themes);
    /** @brief 设置当前选中的主题 @param themeName 主题名称 */
    void setCurrentTheme(const QString& themeName);
    /** @brief 根据索引获取主题名称 @param index 下拉框索引 @return 主题名称 */
    QString themeNameAt(int index) const;
    /** @brief 设置当前选中的语言 @param langCode 语言代码(如"zh_CN"/"en") */
    void setCurrentLanguage(const QString& langCode);
    /** @brief 根据索引获取语言代码 @param index 下拉框索引 @return 语言代码 */
    QString languageCodeAt(int index) const;

signals:
    void displayModeChanged(int index);      ///< 显示模式下拉框索引变更
    void timestampToggled(bool checked);     ///< 时间戳开关切换
    void dirPrefixToggled(bool checked);     ///< 收发方向前缀开关切换
    void clearRequested();                   ///< 清屏按钮点击
    void exportRequested();                  ///< 导出按钮点击
    void bgSettingsRequested();              ///< 背景设置按钮点击
    void themeChanged(int index);            ///< 主题下拉框索引变更
    void languageChanged(int index);         ///< 语言下拉框索引变更
    void terminalLayoutChanged(int index);   ///< 终端布局模式下拉框变更(0=混合,1=左右,2=上下)

private:
    RecordingController* m_recordingController; ///< 录制控制器
    QToolBar* m_toolbar;                     ///< 主工具栏实例
    QComboBox* m_displayModeCombo;           ///< 显示模式下拉框
    QComboBox* m_layoutCombo;                ///< 终端布局模式下拉框
    QComboBox* m_themeCombo;                 ///< 主题选择下拉框
    QComboBox* m_langCombo;                  ///< 语言选择下拉框
    QAction* m_timestampAction;              ///< 时间戳显示开关
    QAction* m_dirPrefixAction;              ///< 收发方向前缀开关
    QAction* m_clearAction;                  ///< 清屏按钮
    QAction* m_exportAction;                 ///< 数据导出按钮
    QAction* m_bgAction;                     ///< 背景设置按钮

    // ---- 统计计数器 ----
    quint64 m_totalDisplayModeChanges = 0;   ///< 累计显示模式切换次数
    quint64 m_totalThemeChanges = 0;         ///< 累计主题切换次数
    quint64 m_totalExports = 0;              ///< 累计导出按钮点击次数
    quint64 m_totalClears = 0;               ///< 累计清屏次数
    quint64 m_totalButtonPresses = 0;        ///< 累计所有按钮点击次数
    quint64 m_totalModeSwitches = 0;         ///< 累计模式切换次数(显示模式+终端布局)
    quint64 m_totalPanelRefreshes = 0;       ///< 累计面板刷新次数
public:
    /** @brief 获取累计显示模式切换次数 @return 切换次数 */
    quint64 totalDisplayModeChanges() const { return m_totalDisplayModeChanges; }
    /** @brief 获取累计主题切换次数 @return 切换次数 */
    quint64 totalThemeChanges() const { return m_totalThemeChanges; }
    /** @brief 获取累计导出按钮点击次数 @return 导出次数 */
    quint64 totalExports() const { return m_totalExports; }
    /** @brief 获取累计清屏次数 @return 清屏次数 */
    quint64 totalClears() const { return m_totalClears; }
    /** @brief 获取累计所有按钮点击次数 @return 点击次数 */
    quint64 totalButtonPresses() const { return m_totalButtonPresses; }
    /** @brief 获取累计模式切换次数(显示模式+终端布局) @return 切换次数 */
    quint64 totalModeSwitches() const { return m_totalModeSwitches; }
    /** @brief 获取累计面板刷新次数 @return 刷新次数 */
    quint64 totalPanelRefreshes() const { return m_totalPanelRefreshes; }
    /** @brief 重置基础统计计数器(显示模式/主题/导出/清屏) */
    void resetToolbarStatistics() { m_totalDisplayModeChanges = 0; m_totalThemeChanges = 0; m_totalExports = 0; m_totalClears = 0; }
    /** @brief 重置所有工具栏统计计数器(含按钮点击/模式切换/面板刷新) */
    void resetStats() { m_totalDisplayModeChanges = 0; m_totalThemeChanges = 0; m_totalExports = 0; m_totalClears = 0; m_totalButtonPresses = 0; m_totalModeSwitches = 0; m_totalPanelRefreshes = 0; }
};

#endif // TOOLBARCONTROLLER_H
