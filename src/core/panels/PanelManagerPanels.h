/**
 * @file PanelManagerPanels.h
 * @brief 面板管理器 - 所有面板的前向声明和成员指针定义
 *
 * 本文件从 PanelManager.h 中提取，避免 PanelManager.h 因面板数量增多超过 200 行限制。
 * 每个 Feature Module 的面板以分组注释组织，便于维护。
 *
 * 新增面板时必须:
 *   1. 在此文件添加前向声明 + 成员指针
 *   2. 在 PanelManagerCreation.cpp 的 createPanels() 中创建实例
 *   3. 在 PanelManagerGetters.cpp 中添加 getter
 *   4. 在 PanelManagerQuery.cpp 的 panelDescriptors() 中登记元数据
 */

#ifndef PANEL_MANAGER_PANELS_H
#define PANEL_MANAGER_PANELS_H

// ==================== 核心 (Core) ====================
class SerialConfigPanel;
class DataStatistics;
class ProtocolView;
class FrameVisualEditor;
class ChartWidget;
class OtaWidget;
class TerminalWidget;
class TerminalSearchBar;
class QuickCommandBar;
class BookmarkWidget;

// ==================== 录制回放 (Recording) ====================
class PlaybackWidget;

// ==================== 仪表盘 (Dashboard) ====================
class DashboardWidget;

// ==================== 终端增强 (Terminal) ====================
class TerminalFilterBar;

// ==================== 连接层 (Connection) ====================
class BleConfigPanel;
class BleGattBrowser;
class CanConfigPanel;
class CanBusMonitor;
class MqttConfigPanel;
class MqttSubscriptionPanel;
class MultiConnectionPanel;
class SpiI2cConfigPanel;
class WsConfigPanel;
class UsbConfigPanel;
class UsbDescriptorViewer;

// ==================== 协议层 (Protocol) ====================
class ProtocolSchemaEditor;
class ModbusConfigPanel;
class ModbusScanWidget;
class SchemaViewer;

// ==================== 调试层 (Debug) ====================
class RttConfigPanel;
class RegisterEditor;
class SignalLineWidget;
class TrafficMonitorWidget;
class TriggerListPanel;

// ==================== 工具层 (Utils) ====================
class ChecksumPanel;
class ConverterPanel;
class TimestampPanel;
class PacketBuilderPanel;
class DataDiffWidget;

// ==================== 系统层 (System) ====================
class PluginConfigPanel;
class ProjectWelcomeDialog;
class DeviceProfilePanel;
class PerformanceOverlay;
class FftWidget;
class ScatterWidget;
class HistogramWidget;

// ==================== 脚本录制 (Script) ====================
class ScriptRecorder;

// ==================== 通用容器 ====================
class BasePanel;

// ==================== 导出辅助 ====================
class ExportDialog;

// ==================== 面板成员指针结构体 ====================
/**
 * @brief 所有面板的成员指针集合
 *
 * 从 PanelManager.h 中提取，避免 PanelManager.h 因面板数量增多超过 200 行限制。
 * 新增面板时在此结构体中添加成员指针，并保持分组注释一致。
 */
struct PanelManagerMembers {
    // --- 核心面板 ---
    SerialConfigPanel* m_serialConfig = nullptr;
    DataStatistics* m_dataStats = nullptr;
    ProtocolView* m_protocolView = nullptr;
    FrameVisualEditor* m_frameEditor = nullptr;
    ChartWidget* m_chartWidget = nullptr;
    OtaWidget* m_otaWidget = nullptr;
    TerminalWidget* m_terminal = nullptr;
    TerminalSearchBar* m_searchBar = nullptr;
    QuickCommandBar* m_quickCmdBar = nullptr;
    BookmarkWidget* m_bookmarkWidget = nullptr;
    // --- 录制回放 ---
    PlaybackWidget* m_playbackWidget = nullptr;
    // --- 仪表盘 ---
    DashboardWidget* m_dashboardWidget = nullptr;
    // --- 终端增强 ---
    TerminalFilterBar* m_terminalFilterBar = nullptr;
    // --- 脚本录制 ---
    ScriptRecorder* m_scriptRecorder = nullptr;
    // --- 连接层 ---
    BleConfigPanel* m_bleConfigPanel = nullptr;
    BleGattBrowser* m_bleGattBrowser = nullptr;
    CanConfigPanel* m_canConfigPanel = nullptr;
    CanBusMonitor* m_canBusMonitor = nullptr;
    MqttConfigPanel* m_mqttConfigPanel = nullptr;
    MqttSubscriptionPanel* m_mqttSubscriptionPanel = nullptr;
    MultiConnectionPanel* m_multiConnectionPanel = nullptr;
    SpiI2cConfigPanel* m_spiI2cConfigPanel = nullptr;
    WsConfigPanel* m_wsConfigPanel = nullptr;
    UsbConfigPanel* m_usbConfigPanel = nullptr;
    UsbDescriptorViewer* m_usbDescriptorViewer = nullptr;
    // --- 协议层 ---
    ProtocolSchemaEditor* m_protocolSchemaEditor = nullptr;
    ModbusConfigPanel* m_modbusConfigPanel = nullptr;
    ModbusScanWidget* m_modbusScanWidget = nullptr;
    SchemaViewer* m_schemaViewer = nullptr;
    // --- 调试层 ---
    RttConfigPanel* m_rttConfigPanel = nullptr;
    RegisterEditor* m_registerEditor = nullptr;
    SignalLineWidget* m_signalLineWidget = nullptr;
    TrafficMonitorWidget* m_trafficMonitorWidget = nullptr;
    TriggerListPanel* m_triggerListPanel = nullptr;
    // --- 图表扩展 ---
    FftWidget* m_fftWidget = nullptr;
    ScatterWidget* m_scatterWidget = nullptr;
    HistogramWidget* m_histogramWidget = nullptr;
    // --- 工具层 ---
    ChecksumPanel* m_checksumPanel = nullptr;
    ConverterPanel* m_converterPanel = nullptr;
    TimestampPanel* m_timestampPanel = nullptr;
    PacketBuilderPanel* m_packetBuilderPanel = nullptr;
    DataDiffWidget* m_dataDiffPanel = nullptr;
    // --- 系统层 ---
    PluginConfigPanel* m_pluginConfigPanel = nullptr;
    ProjectWelcomeDialog* m_projectWelcomeDialog = nullptr;
    DeviceProfilePanel* m_deviceProfilePanel = nullptr;
    PerformanceOverlay* m_performanceOverlay = nullptr;
};

#endif // PANEL_MANAGER_PANELS_H
