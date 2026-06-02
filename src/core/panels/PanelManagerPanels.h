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
 *   3. 在 PanelManager.cpp 的 getters/mappings/allPanels 中注册
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

// ==================== 系统层 (System) ====================
class PluginConfigPanel;
class ProjectWelcomeDialog;
class DeviceProfilePanel;
class PerformanceOverlay;
class FftWidget;
class ScatterWidget;
class HistogramWidget;

// ==================== 导出辅助 ====================
class ExportDialog;

#endif // PANEL_MANAGER_PANELS_H
