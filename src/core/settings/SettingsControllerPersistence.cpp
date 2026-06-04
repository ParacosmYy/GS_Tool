/**
 * @file SettingsControllerPersistence.cpp
 * @brief 设置控制器 - 单项持久化委托方法实现
 *
 * 从 SettingsController.cpp 拆分而来，包含各独立设置项的
 * 保存/加载方法（串口配置、窗口几何、主题、面板索引）。
 */

#include "core/settings/SettingsController.h"
#include "utils/settings/SettingsManager.h"
#include "serial/config/SerialConfigPanel.h"

/**
 * @brief 保存完整串口配置到磁盘
 *
 * 收集 SerialConfigPanel 中所有参数（含 DTR/RTS 线路信号状态），
 * 委托给 SettingsManager 持久化存储。
 * 无 SerialConfigPanel 引用时不执行任何操作。
 */
void SettingsController::saveSerialConfig()
{
    if (!m_serialConfig) return;

    auto& settings = SettingsManager::instance();
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

/**
 * @brief 从磁盘加载串口配置
 * @return 串口配置 QVariantMap，无保存数据时返回空 map
 */
QVariantMap SettingsController::loadSerialConfig() const
{
    return SettingsManager::instance().loadSerialConfig();
}

/**
 * @brief 保存窗口几何信息
 * @param geometry QMainWindow::saveGeometry() 返回的字节数组
 */
void SettingsController::saveWindowGeometry(const QByteArray& geometry)
{
    SettingsManager::instance().saveWindowGeometry(geometry);
}

/**
 * @brief 加载窗口几何信息
 * @return 窗口几何字节数组，无保存数据时返回空 QByteArray
 */
QByteArray SettingsController::loadWindowGeometry() const
{
    return SettingsManager::instance().loadWindowGeometry();
}

/**
 * @brief 保存当前主题名称
 * @param themeName 主题文件名（如 "dark_terminal"）
 */
void SettingsController::saveTheme(const QString& themeName)
{
    SettingsManager::instance().saveTheme(themeName);
}

/**
 * @brief 加载保存的主题名称
 * @return 主题名称字符串，无保存数据时返回默认主题
 */
QString SettingsController::loadTheme() const
{
    return SettingsManager::instance().loadTheme();
}

/**
 * @brief 保存上次活跃的面板索引
 * @param panelIndex 面板索引值
 */
void SettingsController::saveLastPanel(int panelIndex)
{
    SettingsManager::instance().set("session/lastPanel", panelIndex);
}

/**
 * @brief 加载上次活跃的面板索引
 * @return 面板索引值，无保存数据时返回 -1
 */
int SettingsController::loadLastPanel() const
{
    return SettingsManager::instance().get("session/lastPanel", -1).toInt();
}
