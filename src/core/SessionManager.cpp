/**
 * @file SessionManager.cpp
 * @brief 会话管理器实现 - 保存和恢复用户完整工作区状态
 *
 * 一次会话包含: 窗口几何 + 串口配置 + 主题偏好 + 上次面板索引
 * 所有持久化操作委托给 SettingsManager 单例。
 */

#include "core/SessionManager.h"
#include "core/SettingsController.h"
#include "serial/SerialConfigPanel.h"
#include "utils/SettingsManager.h"
#include "ThemeManager.h"

#include <QMainWindow>

/**
 * @brief 构造会话管理器
 * @param mainWindow 主窗口实例，用于保存/恢复窗口几何
 * @param parent 父对象
 */
SessionManager::SessionManager(QMainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_serialConfig(nullptr)
    , m_settingsController(nullptr)
{
}

/** @brief 析构函数 */
SessionManager::~SessionManager()
{
}

/** @brief 注入串口配置面板引用 */
void SessionManager::setSerialConfigPanel(SerialConfigPanel* panel)
{
    m_serialConfig = panel;
}

/** @brief 注入设置控制器引用 */
void SessionManager::setSettingsController(SettingsController* controller)
{
    m_settingsController = controller;
}

/**
 * @brief 保存当前完整工作区到磁盘
 *
 * 保存流程（按顺序）:
 *   1. 窗口几何: 通过 SettingsController 保存主窗口位置和大小
 *   2. 串口配置: 从 SerialConfigPanel 收集全部参数（含 DTR/RTS）
 *   3. 主题: 保存 ThemeManager 中当前活跃的主题名称
 *   4. 上次面板: 保存面板索引值
 *   5. 同步写入磁盘
 */
void SessionManager::saveSession()
{
    auto& settings = SettingsManager::instance();

    // 1. 保存窗口几何（位置和大小）
    if (m_mainWindow && m_settingsController) {
        m_settingsController->saveWindowGeometry(m_mainWindow->saveGeometry());
    }

    // 2. 保存完整串口配置（端口名/波特率/数据位/校验/停止位/流控/DTR/RTS）
    if (m_serialConfig) {
        QVariantMap serialConfig;
        serialConfig["portName"]    = m_serialConfig->currentPortData();
        serialConfig["baudRate"]    = m_serialConfig->currentBaudRate();
        serialConfig["dataBits"]    = m_serialConfig->currentDataBitsIndex();
        serialConfig["parity"]      = m_serialConfig->currentParityIndex();
        serialConfig["stopBits"]    = m_serialConfig->currentStopBitsIndex();
        serialConfig["flowControl"] = m_serialConfig->currentFlowControlIndex();
        serialConfig["dtr"]         = m_serialConfig->dtrEnabled();
        serialConfig["rts"]         = m_serialConfig->rtsEnabled();
        settings.saveSerialConfig(serialConfig);
    }

    // 3. 保存当前主题名称
    if (m_settingsController) {
        m_settingsController->saveTheme(ThemeManager::instance().currentTheme());
    }

    // 5. 同步写入磁盘，确保数据落盘
    settings.sync();
}

/**
 * @brief 从磁盘恢复上次保存的工作区
 *
 * 恢复流程:
 *   1. 窗口几何 → 通过 SettingsController 恢复窗口位置和大小
 *   2. 串口配置 → 从 SettingsManager 读取并恢复到 SerialConfigPanel
 *   3. 读取上次面板索引并返回给调用方
 *
 * 注意: 主题恢复由 SettingsController::loadSettings() 统一处理，
 *       本方法不再重复执行主题加载。
 *
 * @return 上次活跃的面板索引，无保存数据时返回 -1
 */
int SessionManager::loadSession()
{
    auto& settings = SettingsManager::instance();

    // 1. 恢复窗口几何（位置和大小）
    if (m_mainWindow && m_settingsController) {
        QByteArray geometry = m_settingsController->loadWindowGeometry();
        if (!geometry.isEmpty()) {
            m_mainWindow->restoreGeometry(geometry);
        }
    }

    // 2. 恢复串口配置到配置面板（端口/波特率/数据位/校验/流控/DTR/RTS）
    if (m_serialConfig) {
        QVariantMap serialConfig = settings.loadSerialConfig();
        if (!serialConfig.isEmpty()) {
            m_serialConfig->restoreConfig(serialConfig);
        }
    }

    // 3. 读取上次面板索引
    int lastPanel = -1;
    if (m_settingsController) {
        lastPanel = m_settingsController->loadLastPanel();
    }

    return lastPanel;
}
