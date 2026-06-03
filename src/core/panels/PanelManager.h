/**
 * @file PanelManager.h
 * @brief 面板管理器 - 管理所有面板的创建、缓存和切换
 *
 * 面板的前向声明和成员指针定义在 PanelManagerPanels.h 中，
 * 以控制本文件行数在 200 行以内。
 */

#ifndef PANEL_MANAGER_H
#define PANEL_MANAGER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include "core/navigation/NavigationController.h"
#include "core/panels/PanelManagerPanels.h"
#include "core/widgets/BasePanel.h"
#include "core/widgets/ScriptRecorder.h"

class OtaManager;
class TerminalModel;

/**
 * @brief 面板管理器 - 统一创建和管理所有功能面板 widget
 *
 * 职责:
 *   1. 集中创建所有功能面板（串口配置、终端、数据统计、协议解析、波形图、OTA等）
 *   2. 提供统一的 Getter 接口供 MainWindow 和各 Controller 获取面板指针
 *   3. 生成面板映射表供 NavigationController 构建导航树
 *   4. 生成可切换面板列表供 NavigationController 执行面板隐藏/显示
 *
 * 设计目的:
 *   从 MainWindow 中提取面板创建逻辑，使 MainWindow 只负责 UI 布局组装和信号连接，
 *   不再直接持有和创建具体的面板 widget，降低 MainWindow 的代码量和职责。
 *
 * 协作关系:
 *   - MainWindow: 调用 createPanels() 创建面板，通过 Getter 获取面板指针用于布局和信号连接
 *   - NavigationController: 使用 panelMappings() 构建导航树，使用 allPanels() 切换面板
 */
class PanelManager : public QObject, private PanelManagerMembers {
    Q_OBJECT

public:
    explicit PanelManager(QObject* parent = nullptr);
    ~PanelManager() override = default;
    PanelManager(const PanelManager&) = delete;
    PanelManager& operator=(const PanelManager&) = delete;

    /**
     * @brief 创建所有面板 widget（调用一次，在 MainWindow::setupUI 中）
     * @param otaManager OTA 管理器指针
     * @param terminalModel 终端数据模型指针
     */
    void createPanels(OtaManager* otaManager, TerminalModel* terminalModel);

    // ==================== 面板 Getter ====================

    // --- 核心面板 ---
    SerialConfigPanel* serialConfig() const;
    DataStatistics* dataStats() const;
    ProtocolView* protocolView() const;
    FrameVisualEditor* frameEditor() const;
    ChartWidget* chartWidget() const;
    OtaWidget* otaWidget() const;
    TerminalWidget* terminal() const;
    TerminalSearchBar* searchBar() const;
    QuickCommandBar* quickCmdBar() const;
    BookmarkWidget* bookmarkWidget() const;

    // --- 录制回放 ---
    PlaybackWidget* playbackWidget() const;

    // --- 仪表盘 ---
    DashboardWidget* dashboardWidget() const;

    // --- 终端增强 ---
    TerminalFilterBar* terminalFilterBar() const;

    // --- 脚本录制 ---
    ScriptRecorder* scriptRecorder() const;

    // --- 连接层 ---
    BleConfigPanel* bleConfigPanel() const;
    BleGattBrowser* bleGattBrowser() const;
    CanConfigPanel* canConfigPanel() const;
    CanBusMonitor* canBusMonitor() const;
    MqttConfigPanel* mqttConfigPanel() const;
    MqttSubscriptionPanel* mqttSubscriptionPanel() const;
    MultiConnectionPanel* multiConnectionPanel() const;
    SpiI2cConfigPanel* spiI2cConfigPanel() const;
    WsConfigPanel* wsConfigPanel() const;
    UsbConfigPanel* usbConfigPanel() const;
    UsbDescriptorViewer* usbDescriptorViewer() const;

    // --- 协议层 ---
    ProtocolSchemaEditor* protocolSchemaEditor() const;
    ModbusConfigPanel* modbusConfigPanel() const;
    ModbusScanWidget* modbusScanWidget() const;
    SchemaViewer* schemaViewer() const;

    // --- 调试层 ---
    RttConfigPanel* rttConfigPanel() const;
    RegisterEditor* registerEditor() const;
    SignalLineWidget* signalLineWidget() const;
    TrafficMonitorWidget* trafficMonitorWidget() const;
    TriggerListPanel* triggerListPanel() const;

    // --- 图表扩展 ---
    FftWidget* fftWidget() const;
    ScatterWidget* scatterWidget() const;
    HistogramWidget* histogramWidget() const;

    // --- 工具层 ---
    ChecksumPanel* checksumPanel() const;
    ConverterPanel* converterPanel() const;
    TimestampPanel* timestampPanel() const;
    PacketBuilderPanel* packetBuilderPanel() const;
    DataDiffWidget* dataDiffPanel() const;

    // --- 系统层 ---
    PluginConfigPanel* pluginConfigPanel() const;
    ProjectWelcomeDialog* projectWelcomeDialog() const;
    DeviceProfilePanel* deviceProfilePanel() const;
    PerformanceOverlay* performanceOverlay() const;

    // ==================== 映射表接口 ====================
    QVector<NavPanelMapping> panelMappings() const;
    QVector<QWidget*> allPanels() const;

    // ==================== BasePanel包装器 ====================
    BasePanel* wrapper(QWidget* rawPanel) const;  ///< 获取面板的BasePanel包装器
    void wrapPanels();                            ///< 创建所有BasePanel包装器

private:
    // --- BasePanel包装器 ---
    QMap<QWidget*, BasePanel*> m_wrappers;  ///< 原始面板→BasePanel包装器映射
};

#endif // PANEL_MANAGER_H
