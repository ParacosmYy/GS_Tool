#include "SettingsController.h"
#include "ToolbarController.h"
#include "utils/SettingsManager.h"
#include "ThemeManager.h"
#include "serial/SerialConfigPanel.h"
#include "Constants.h"

#include <QMainWindow>
#include <QStatusBar>

SettingsController::SettingsController(QMainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_toolbarController(nullptr)
    , m_serialConfig(nullptr)
{
}

SettingsController::~SettingsController()
{
}

void SettingsController::setToolbarController(ToolbarController* controller)
{
    m_toolbarController = controller;
}

void SettingsController::setSerialConfigPanel(SerialConfigPanel* panel)
{
    m_serialConfig = panel;
}

void SettingsController::loadSettings()
{
    auto& settings = SettingsManager::instance();

    // 恢复窗口几何
    QByteArray geometry = settings.loadWindowGeometry();
    if (!geometry.isEmpty()) {
        m_mainWindow->restoreGeometry(geometry);
    }

    // 恢复主题
    QString savedTheme = settings.loadTheme();
    if (ThemeManager::instance().loadTheme(savedTheme)) {
        // 通过ToolbarController同步主题下拉框选中项
        if (m_toolbarController) {
            m_toolbarController->setCurrentTheme(savedTheme);
        }
    }

    // 恢复串口配置到配置面板
    if (m_serialConfig) {
        QVariantMap serialConfig = settings.loadSerialConfig();
        if (!serialConfig.isEmpty()) {
            m_serialConfig->restoreConfig(serialConfig);
        }
    }

    // 恢复语言选择
    if (m_toolbarController) {
        QString savedLang = settings.loadLanguage();
        m_toolbarController->setCurrentLanguage(savedLang);
    }
}

void SettingsController::saveSettings()
{
    auto& settings = SettingsManager::instance();

    // 保存窗口几何
    settings.saveWindowGeometry(m_mainWindow->saveGeometry());

    // 保存当前主题
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

    settings.sync();
}

void SettingsController::onThemeChanged(int index)
{
    if (!m_toolbarController) return;

    QString themeName = m_toolbarController->themeNameAt(index);
    if (!themeName.isEmpty()) {
        ThemeManager::instance().loadTheme(themeName);
    }
}

void SettingsController::onLanguageChanged(int index)
{
    if (!m_toolbarController) return;

    QString langCode = m_toolbarController->languageCodeAt(index);
    SettingsManager::instance().saveLanguage(langCode);

    // 用硬编码字符串而非tr()，因为翻译此刻尚未生效
    if (langCode == Language::ENGLISH) {
        m_mainWindow->statusBar()->showMessage(
            QStringLiteral("Language changed to English, restart to apply"), 3000);
    } else {
        m_mainWindow->statusBar()->showMessage(
            QStringLiteral("语言已切换为中文，重启后生效"), 3000);
    }
}
