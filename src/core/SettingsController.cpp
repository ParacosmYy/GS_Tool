/**
 * @file SettingsController.cpp
 * @brief 设置控制器实现 - 应用设置的持久化加载与保存
 *
 * 设置项: 窗口几何位置/大小、主题名称、串口配置参数、语言偏好
 * 底层持久化委托给 SettingsManager 单例（QSettings 封装）
 */

#include "SettingsController.h"
#include "ToolbarController.h"
#include "utils/SettingsManager.h"
#include "ThemeManager.h"
#include "serial/SerialConfigPanel.h"
#include "Constants.h"

#include <QMainWindow>
#include <QStatusBar>

/**
 * @brief 构造设置控制器
 * @param mainWindow 主窗口实例，用于恢复/保存窗口几何
 * @param parent 父对象
 */
SettingsController::SettingsController(QMainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_toolbarController(nullptr)
    , m_serialConfig(nullptr)
{
}

/** @brief 析构函数 */
SettingsController::~SettingsController()
{
}

/** @brief 注入工具栏控制器引用 */
void SettingsController::setToolbarController(ToolbarController* controller)
{
    m_toolbarController = controller;
}

/** @brief 注入串口配置面板引用 */
void SettingsController::setSerialConfigPanel(SerialConfigPanel* panel)
{
    m_serialConfig = panel;
}

/**
 * @brief 恢复所有保存的设置
 * 加载顺序: 窗口几何 → 主题 → 串口配置 → 语言
 */
void SettingsController::loadSettings()
{
    auto& settings = SettingsManager::instance();

    // 恢复窗口几何（位置和大小）
    QByteArray geometry = settings.loadWindowGeometry();
    if (!geometry.isEmpty()) {
        m_mainWindow->restoreGeometry(geometry);
    }

    // 恢复主题: 加载主题文件 + 同步工具栏下拉框选中项
    QString savedTheme = settings.loadTheme();
    if (ThemeManager::instance().loadTheme(savedTheme)) {
        if (m_toolbarController) {
            m_toolbarController->setCurrentTheme(savedTheme);
        }
    }

    // 恢复串口配置到配置面板（端口/波特率/数据位/校验/流控）
    if (m_serialConfig) {
        QVariantMap serialConfig = settings.loadSerialConfig();
        if (!serialConfig.isEmpty()) {
            m_serialConfig->restoreConfig(serialConfig);
        }
    }

    // 恢复语言选择到工具栏下拉框
    if (m_toolbarController) {
        QString savedLang = settings.loadLanguage();
        m_toolbarController->setCurrentLanguage(savedLang);
    }
}

/**
 * @brief 持久化当前设置到磁盘
 * 保存: 窗口几何 → 主题名称 → 串口配置参数 → 同步写入磁盘
 */
void SettingsController::saveSettings()
{
    auto& settings = SettingsManager::instance();

    // 保存窗口几何（位置和大小）
    settings.saveWindowGeometry(m_mainWindow->saveGeometry());

    // 保存当前主题名称
    settings.saveTheme(ThemeManager::instance().currentTheme());

    // 保存串口配置（从配置面板获取当前值）
    if (m_serialConfig) {
        QVariantMap serialConfig;
        serialConfig["portName"] = m_serialConfig->currentPortData();
        serialConfig["baudRate"] = m_serialConfig->currentBaudRate();
        serialConfig["dataBits"] = m_serialConfig->currentDataBitsIndex();
        serialConfig["parity"] = m_serialConfig->currentParityIndex();
        serialConfig["stopBits"] = m_serialConfig->currentStopBitsIndex();
        serialConfig["flowControl"] = m_serialConfig->currentFlowControlIndex();
        settings.saveSerialConfig(serialConfig);
    }

    // 同步写入磁盘
    settings.sync();
}

/**
 * @brief 主题下拉框选中项变更处理
 * 从 ToolbarController 获取主题名称并应用到 ThemeManager
 * @param index 下拉框选中索引
 */
void SettingsController::onThemeChanged(int index)
{
    if (!m_toolbarController) return;

    QString themeName = m_toolbarController->themeNameAt(index);
    if (!themeName.isEmpty()) {
        ThemeManager::instance().loadTheme(themeName);
    }
}

/**
 * @brief 语言下拉框选中项变更处理
 * 保存语言偏好到 SettingsManager，并提示用户重启生效
 * 注意: Qt 翻译需要在应用启动时安装，运行时切换需要重启
 * @param index 下拉框选中索引
 */
void SettingsController::onLanguageChanged(int index)
{
    if (!m_toolbarController) return;

    QString langCode = m_toolbarController->languageCodeAt(index);
    SettingsManager::instance().saveLanguage(langCode);

    // 用硬编码字符串而非 tr()，因为翻译此刻尚未生效
    if (langCode == Language::ENGLISH) {
        m_mainWindow->statusBar()->showMessage(
            QStringLiteral("Language changed to English, restart to apply"), 3000);
    } else {
        m_mainWindow->statusBar()->showMessage(
            QStringLiteral("语言已切换为中文，重启后生效"), 3000);
    }
}
