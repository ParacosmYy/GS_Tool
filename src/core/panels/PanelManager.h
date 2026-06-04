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

class OtaManager;
class TerminalModel;

/** @brief 面板管理器 - 统一创建和管理所有功能面板widget。从MainWindow提取面板创建逻辑，提供Getter/映射表/可切换面板列表 */
class PanelManager : public QObject, private PanelManagerMembers {
    Q_OBJECT

public:
    /** @brief 构造面板管理器 @param parent 父对象 */
    explicit PanelManager(QObject* parent = nullptr);
    /** @brief 析构(QObject父子树自动回收) */
    ~PanelManager() override = default;
    PanelManager(const PanelManager&) = delete; ///< 禁止拷贝
    PanelManager& operator=(const PanelManager&) = delete; ///< 禁止赋值
    /** @brief 创建所有面板(调用一次) @param otaManager OTA管理器实例 @param terminalModel 终端数据模型 */
    void createPanels(OtaManager* otaManager, TerminalModel* terminalModel);

    // ==================== 面板 Getter ====================

    // --- 核心面板 ---
    SerialConfigPanel* serialConfig() const;        ///< 获取串口配置面板
    DataStatistics* dataStats() const;              ///< 获取数据统计面板
    ProtocolView* protocolView() const;             ///< 获取协议解析视图面板
    FrameVisualEditor* frameEditor() const;         ///< 获取帧可视化编辑器面板
    ChartWidget* chartWidget() const;               ///< 获取波形图表面板
    OtaWidget* otaWidget() const;                   ///< 获取OTA升级面板
    TerminalWidget* terminal() const;               ///< 获取终端显示面板
    TerminalSearchBar* searchBar() const;           ///< 获取终端搜索栏
    QuickCommandBar* quickCmdBar() const;           ///< 获取快捷指令栏
    BookmarkWidget* bookmarkWidget() const;         ///< 获取书签面板

    // --- 录制回放 ---
    PlaybackWidget* playbackWidget() const;         ///< 获取数据录制回放面板

    // --- 仪表盘 ---
    DashboardWidget* dashboardWidget() const;       ///< 获取仪表盘面板

    // --- 终端增强 ---
    TerminalFilterBar* terminalFilterBar() const;   ///< 获取终端过滤栏

    // --- 脚本录制 ---
    ScriptRecorder* scriptRecorder() const;         ///< 获取脚本录制器

    // --- 连接层 ---
    BleConfigPanel* bleConfigPanel() const;         ///< 获取BLE配置面板
    BleGattBrowser* bleGattBrowser() const;         ///< 获取BLE GATT浏览器
    CanConfigPanel* canConfigPanel() const;         ///< 获取CAN配置面板
    CanBusMonitor* canBusMonitor() const;           ///< 获取CAN总线监控器
    MqttConfigPanel* mqttConfigPanel() const;       ///< 获取MQTT配置面板
    MqttSubscriptionPanel* mqttSubscriptionPanel() const; ///< 获取MQTT订阅面板
    MultiConnectionPanel* multiConnectionPanel() const;   ///< 获取多连接管理面板
    SpiI2cConfigPanel* spiI2cConfigPanel() const;   ///< 获取SPI/I2C桥接配置面板
    WsConfigPanel* wsConfigPanel() const;           ///< 获取WebSocket配置面板
    UsbConfigPanel* usbConfigPanel() const;         ///< 获取USB配置面板
    UsbDescriptorViewer* usbDescriptorViewer() const; ///< 获取USB描述符查看器

    // --- 协议层 ---
    ProtocolSchemaEditor* protocolSchemaEditor() const; ///< 获取协议Schema编辑器
    ModbusConfigPanel* modbusConfigPanel() const;   ///< 获取Modbus配置面板
    ModbusScanWidget* modbusScanWidget() const;     ///< 获取Modbus扫描面板
    SchemaViewer* schemaViewer() const;             ///< 获取Schema查看器

    // --- 调试层 ---
    RttConfigPanel* rttConfigPanel() const;         ///< 获取RTT配置面板
    RegisterEditor* registerEditor() const;         ///< 获取寄存器编辑器
    SignalLineWidget* signalLineWidget() const;     ///< 获取信号线监控面板
    TrafficMonitorWidget* trafficMonitorWidget() const; ///< 获取流量监控面板
    TriggerListPanel* triggerListPanel() const;     ///< 获取触发器列表面板

    // --- 图表扩展 ---
    FftWidget* fftWidget() const;                   ///< 获取FFT频谱分析面板
    ScatterWidget* scatterWidget() const;           ///< 获取散点图面板
    HistogramWidget* histogramWidget() const;       ///< 获取直方图面板

    // --- 工具层 ---
    ChecksumPanel* checksumPanel() const;           ///< 获取校验和计算面板
    ConverterPanel* converterPanel() const;         ///< 获取数据转换面板
    TimestampPanel* timestampPanel() const;         ///< 获取时间戳面板
    PacketBuilderPanel* packetBuilderPanel() const; ///< 获取数据包构建面板
    DataDiffWidget* dataDiffPanel() const;          ///< 获取数据对比面板

    // --- 系统层 ---
    PluginConfigPanel* pluginConfigPanel() const;   ///< 获取插件配置面板
    ProjectWelcomeDialog* projectWelcomeDialog() const; ///< 获取工程欢迎对话框
    DeviceProfilePanel* deviceProfilePanel() const; ///< 获取设备配置面板
    PerformanceOverlay* performanceOverlay() const; ///< 获取性能监控浮层

    // ==================== 映射表接口 ====================
    /** @brief 生成面板映射表，供NavigationController构建导航树使用 */
    QVector<NavPanelMapping> panelMappings() const;
    /** @brief 获取所有可切换面板widget列表 */
    QVector<QWidget*> allPanels() const;

    // ==================== BasePanel包装器 ====================
    /** @brief 获取面板的BasePanel包装器 @param rawPanel 原始面板指针 @return BasePanel包装器，不存在返回nullptr */
    BasePanel* wrapper(QWidget* rawPanel) const;
    /** @brief 创建所有BasePanel包装器 */
    void wrapPanels();

    // ==================== 响应式布局 ====================

    /** @brief 设置紧凑模式(小窗口隐藏非关键面板扩展区域) @param compact true=启用紧凑模式 */
    void setCompactMode(bool compact);
    /** @brief 查询是否处于紧凑模式 @return true=紧凑模式 */
    bool isCompactMode() const;

    // ==================== 面板统计 ====================
    /** @brief 通知面板切换(由NavigationController调用) @param visibleCount 当前可见面板数 */
    void onPanelSwitched(int visibleCount);
    quint64 totalPanelSwitches() const;      ///< @return 累计面板切换次数
    quint64 totalPanelsCreated() const;      ///< @return 累计创建面板总数
    quint64 maxConcurrentPanels() const;     ///< @return 历史最大并发面板数
    quint64 totalPanelCreations() const;     ///< @return 累计面板创建次数(wrapPanels逐个创建)
    quint64 totalPanelDeletions() const;     ///< @return 累计面板删除次数
    quint64 totalActivePanelsTracked() const; ///< @return 累计活跃面板追踪次数
    void resetStats();                       ///< 重置所有统计计数器

private:
    // --- 面板工厂辅助方法(实现见PanelManagerFactory.cpp) ---
    /** @brief 创建连接层面板(BLE/CAN/MQTT/TCP/SPI/I2C/WS/USB) @param parent 面板父控件 */
    void createConnectionPanels(QWidget* parent);
    /** @brief 创建协议层面板(自定义协议/Modbus/Protobuf) @param parent 面板父控件 */
    void createProtocolPanels(QWidget* parent);
    /** @brief 创建调试层面板(RTT/寄存器/信号线/流量/触发器) @param parent 面板父控件 */
    void createDebugPanels(QWidget* parent);
    /** @brief 创建图表扩展面板(FFT/散点/直方图) @param parent 面板父控件 */
    void createChartExtensionPanels(QWidget* parent);
    /** @brief 创建工具层面板(校验/转换/时间戳/数据包/对比) @param parent 面板父控件 */
    void createToolPanels(QWidget* parent);
    /** @brief 创建系统层面板(插件/项目/设备/性能) @param parent 面板父控件 */
    void createSystemPanels(QWidget* parent);

    // --- BasePanel包装器 ---
    QMap<QWidget*, BasePanel*> m_wrappers;  ///< 原始面板→BasePanel包装器映射
    bool m_compactMode = false;             ///< 紧凑模式标志

    // --- 统计计数器 ---
    quint64 m_totalPanelSwitches = 0;       ///< 累计面板切换次数
    quint64 m_totalPanelsCreated = 0;       ///< 累计创建面板总数
    quint64 m_maxConcurrentPanels = 0;      ///< 历史最大并发面板数
    quint64 m_totalPanelCreations = 0;      ///< 累计面板创建次数(wrapPanels逐个创建)
    quint64 m_totalPanelDeletions = 0;      ///< 累计面板删除次数
    quint64 m_totalActivePanelsTracked = 0; ///< 累计活跃面板追踪次数
};

#endif // PANEL_MANAGER_H
