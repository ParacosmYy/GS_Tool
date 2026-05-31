#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H

#include <QObject>

class QMainWindow;
class SerialConfigPanel;
class ToolbarController;

/**
 * @brief 设置控制器 - 管理应用设置的持久化加载与保存
 *
 * 职责:
 *   1. 加载保存的设置（窗口几何、主题、串口配置、语言偏好）
 *   2. 在窗口关闭时保存当前设置到磁盘
 *   3. 响应运行时的主题/语言变更事件
 *   4. 通过 ToolbarController 接口同步主题/语言下拉框状态
 *
 * 设计模式:
 *   - 委托模式: 实际的持久化操作委托给 SettingsManager 单例
 *   - 观察者模式: 通过 Qt 信号/槽响应主题/语言变更
 *
 * 协作关系:
 *   - SettingsManager: 底层配置持久化单例（QSettings 封装）
 *   - ThemeManager: 主题切换引擎
 *   - ToolbarController: 提供主题/语言下拉框的索引到名称映射
 *   - SerialConfigPanel: 串口配置参数的保存/恢复
 *   - MainWindow: 持有此控制器，在构造时加载设置，关闭时保存设置
 */
class SettingsController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造设置控制器
     * @param mainWindow 主窗口实例，用于恢复窗口几何
     * @param parent 父对象
     */
    explicit SettingsController(QMainWindow* mainWindow, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~SettingsController() override;

    /**
     * @brief 注入工具栏控制器引用
     * 用于同步主题/语言下拉框的选中项
     * @param controller 工具栏控制器指针
     */
    void setToolbarController(ToolbarController* controller);

    /**
     * @brief 注入串口配置面板引用
     * 用于串口配置参数的保存/恢复
     * @param panel 串口配置面板指针
     */
    void setSerialConfigPanel(SerialConfigPanel* panel);

    /**
     * @brief 恢复所有保存的设置
     * 加载顺序: 窗口几何 → 主题 → 串口配置 → 语言
     */
    void loadSettings();

    /**
     * @brief 持久化当前设置到磁盘
     * 保存: 窗口几何 → 主题名称 → 串口配置参数
     */
    void saveSettings();

public slots:
    /**
     * @brief 主题下拉框选中项变更处理
     * 从 ToolbarController 获取主题名称并应用到 ThemeManager
     * @param index 下拉框选中索引
     */
    void onThemeChanged(int index);

    /**
     * @brief 语言下拉框选中项变更处理
     * 保存语言偏好到 SettingsManager，并提示用户重启生效
     * @param index 下拉框选中索引
     */
    void onLanguageChanged(int index);

private:
    /** @brief 主窗口实例，用于恢复/保存窗口几何位置和大小 */
    QMainWindow* m_mainWindow;

    /** @brief 工具栏控制器，用于获取主题/语言名称和同步下拉框状态 */
    ToolbarController* m_toolbarController;

    /** @brief 串口配置面板，用于保存/恢复串口参数 */
    SerialConfigPanel* m_serialConfig;
};

#endif // SETTINGSCONTROLLER_H
