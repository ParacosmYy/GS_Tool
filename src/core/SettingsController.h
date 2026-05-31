#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H

#include <QObject>

class QMainWindow;
class SerialConfigPanel;
class ToolbarController;

// 设置控制器 - 从MainWindow中提取的设置管理逻辑
// 负责加载/保存窗口几何、主题、串口配置、语言偏好
// 通过ToolbarController接口同步主题/语言下拉框状态
class SettingsController : public QObject {
    Q_OBJECT

public:
    explicit SettingsController(QMainWindow* mainWindow, QObject* parent = nullptr);
    ~SettingsController() override;

    // 注入工具栏控制器引用（用于同步主题/语言下拉框）
    void setToolbarController(ToolbarController* controller);

    // 注入串口配置面板引用（用于配置保存/恢复）
    void setSerialConfigPanel(SerialConfigPanel* panel);

    // 恢复所有保存的设置（窗口几何、主题、串口配置、语言）
    void loadSettings();

    // 持久化当前设置到磁盘（窗口几何、主题、串口配置）
    void saveSettings();

public slots:
    // 主题下拉框选中项变更 - 应用主题
    void onThemeChanged(int index);

    // 语言下拉框选中项变更 - 保存语言偏好
    void onLanguageChanged(int index);

private:
    QMainWindow* m_mainWindow;
    ToolbarController* m_toolbarController;
    SerialConfigPanel* m_serialConfig;
};

#endif // SETTINGSCONTROLLER_H
