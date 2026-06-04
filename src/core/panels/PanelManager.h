/** @file PanelManager.h @brief 面板管理器 - 管理所有面板的创建、缓存和切换。面板前向声明和成员指针在PanelManagerPanels.h中 */
#ifndef PANEL_MANAGER_H
#define PANEL_MANAGER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include "core/navigation/NavigationController.h"
#include "core/panels/PanelManagerPanels.h"
#include "core/widgets/BasePanel.h"
#include "core/widgets/ScriptRecorder.h"

class OtaManager; class TerminalModel;

/** @brief 面板管理器 - 统一创建和管理所有功能面板widget */
class PanelManager : public QObject, private PanelManagerMembers {
    Q_OBJECT

public:
    explicit PanelManager(QObject* parent = nullptr);
    ~PanelManager() override = default;
    PanelManager(const PanelManager&) = delete;
    PanelManager& operator=(const PanelManager&) = delete;
    void createPanels(OtaManager* otaManager, TerminalModel* terminalModel);

    // ==================== 面板 Getter ====================
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
    PlaybackWidget* playbackWidget() const;
    DashboardWidget* dashboardWidget() const;
    TerminalFilterBar* terminalFilterBar() const;
    ScriptRecorder* scriptRecorder() const;
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
    ProtocolSchemaEditor* protocolSchemaEditor() const;
    ModbusConfigPanel* modbusConfigPanel() const;
    ModbusScanWidget* modbusScanWidget() const;
    SchemaViewer* schemaViewer() const;
    RttConfigPanel* rttConfigPanel() const;
    RegisterEditor* registerEditor() const;
    SignalLineWidget* signalLineWidget() const;
    TrafficMonitorWidget* trafficMonitorWidget() const;
    TriggerListPanel* triggerListPanel() const;
    FftWidget* fftWidget() const;
    ScatterWidget* scatterWidget() const;
    HistogramWidget* histogramWidget() const;
    ChecksumPanel* checksumPanel() const;
    ConverterPanel* converterPanel() const;
    TimestampPanel* timestampPanel() const;
    PacketBuilderPanel* packetBuilderPanel() const;
    DataDiffWidget* dataDiffPanel() const;
    PluginConfigPanel* pluginConfigPanel() const;
    ProjectWelcomeDialog* projectWelcomeDialog() const;
    DeviceProfilePanel* deviceProfilePanel() const;
    PerformanceOverlay* performanceOverlay() const;

    QVector<NavPanelMapping> panelMappings() const; ///< 生成面板映射表
    QVector<QWidget*> allPanels() const;            ///< 获取所有可切换面板列表
    BasePanel* wrapper(QWidget* rawPanel) const;    ///< 获取面板的BasePanel包装器
    void wrapPanels();                              ///< 创建所有BasePanel包装器

    void setCompactMode(bool compact);              ///< 设置紧凑模式
    bool isCompactMode() const;                     ///< 查询紧凑模式

    // ---- 面板统计 ----
    void onPanelSwitched(int visibleCount);
    quint64 totalPanelSwitches() const;
    quint64 totalPanelsCreated() const;
    quint64 maxConcurrentPanels() const;
    quint64 totalPanelCreations() const;
    quint64 totalPanelDeletions() const;
    quint64 totalActivePanelsTracked() const;
    quint64 totalPanelRegisters() const;
    quint64 totalPanelUnregisters() const;
    quint64 totalCategoryExpands() const;
    void resetStats();

private:
    void createConnectionPanels(QWidget* parent);
    void createProtocolPanels(QWidget* parent);
    void createDebugPanels(QWidget* parent);
    void createChartExtensionPanels(QWidget* parent);
    void createToolPanels(QWidget* parent);
    void createSystemPanels(QWidget* parent);

    QMap<QWidget*, BasePanel*> m_wrappers;
    bool m_compactMode = false;
    quint64 m_totalPanelSwitches = 0, m_totalPanelsCreated = 0, m_maxConcurrentPanels = 0;
    quint64 m_totalPanelCreations = 0, m_totalPanelDeletions = 0, m_totalActivePanelsTracked = 0;
    quint64 m_totalPanelRegisters = 0, m_totalPanelUnregisters = 0, m_totalCategoryExpands = 0;
};

#endif // PANEL_MANAGER_H
