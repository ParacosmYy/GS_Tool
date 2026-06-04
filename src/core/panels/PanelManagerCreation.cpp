/**
 * @file PanelManagerCreation.cpp
 * @brief 面板管理器 - createPanels() 方法实现
 *
 * 集中创建所有功能面板 widget，每个面板设置 objectName（QSS 选择器）和初始可见性。
 * 从 PanelManager.cpp 中分离以控制单文件行数 ≤ 500 行。
 * 连接层/协议层/调试层/图表扩展/工具层/系统层的创建逻辑
 * 委托给 PanelManagerFactory.cpp 中的工厂辅助方法。
 */

#include "core/panels/PanelManager.h"

// ---- 核心 ----
#include "serial/config/SerialConfigPanel.h"
#include "serial/data/DataStatistics.h"
#include "serial/commands/QuickCommandBar.h"
#include "protocol/view/ProtocolView.h"
#include "protocol/editor/FrameVisualEditor.h"
#include "chart/widget/ChartWidget.h"
#include "ota/manager/OtaManager.h"
#include "ota/widget/OtaWidget.h"
#include "terminal/widget/TerminalWidget.h"
#include "terminal/model/TerminalModel.h"
#include "terminal/search/TerminalSearchBar.h"
#include "serial/data/BookmarkWidget.h"

// ---- 录制回放 / 仪表盘 / 终端增强 ----
#include "core/recording/PlaybackWidget.h"
#include "dashboard/DashboardWidget.h"
#include "terminal/filter/TerminalFilterBar.h"
#include "core/widgets/ScriptRecorder.h"

/** @brief 创建所有面板widget，在MainWindow::setupUI中调用一次，所有面板初始状态为隐藏由NavigationController按需显示 @param otaManager OTA管理器指针，OtaWidget构造时注入 @param terminalModel 终端数据模型指针，TerminalWidget需要设置此模型 */
void PanelManager::createPanels(OtaManager* otaManager, TerminalModel* terminalModel)
{
    QWidget* widgetParent = qobject_cast<QWidget*>(parent());

    // ============================================================
    //  核心面板 (Core) — 串口/协议/图表/终端等基础功能
    // ============================================================

    m_serialConfig = new SerialConfigPanel(widgetParent);           // 串口配置
    m_serialConfig->setObjectName("serialConfigPanel");
    m_serialConfig->setVisible(false);

    m_dataStats = new DataStatistics(widgetParent);                 // 数据统计
    m_dataStats->setObjectName("dataStatsPanel");
    m_dataStats->setVisible(false);

    m_protocolView = new ProtocolView(widgetParent);                // 协议视图
    m_protocolView->setObjectName("protocolViewPanel");
    m_protocolView->setVisible(false);

    m_frameEditor = new FrameVisualEditor(widgetParent);            // 帧编辑器
    m_frameEditor->setObjectName("frameEditorPanel");
    m_frameEditor->setVisible(false);

    m_chartWidget = new ChartWidget(widgetParent);                  // 波形图
    m_chartWidget->setObjectName("chartWidgetPanel");
    m_chartWidget->setVisible(false);

    m_otaWidget = new OtaWidget(otaManager, widgetParent);          // OTA升级
    m_otaWidget->setObjectName("otaWidgetPanel");
    m_otaWidget->setVisible(false);

    m_terminal = new TerminalWidget(widgetParent);                  // 终端（默认可见）
    m_terminal->setObjectName("terminalPanel");
    m_terminal->setModel(terminalModel);

    m_searchBar = new TerminalSearchBar(widgetParent);              // 搜索栏
    m_searchBar->setObjectName("searchBarPanel");
    m_searchBar->setVisible(false);

    m_quickCmdBar = new QuickCommandBar(widgetParent);              // 快捷指令
    m_quickCmdBar->setObjectName("quickCmdBarPanel");
    m_quickCmdBar->setCommands({
        {"AT",     "AT\r\n",           false},
        {"Reset",  "AA 55 01 00 FE",   true},
        {"Status", "AT+STATUS?\r\n",   false}
    });
    m_quickCmdBar->setVisible(false);

    m_bookmarkWidget = new BookmarkWidget(widgetParent);            // 书签
    m_bookmarkWidget->setObjectName("bookmarkWidgetPanel");
    m_bookmarkWidget->setVisible(false);

    // ============================================================
    //  录制回放 (Recording)
    // ============================================================

    m_playbackWidget = new PlaybackWidget(widgetParent);            // F1 录制回放
    m_playbackWidget->setObjectName("playbackWidgetPanel");
    m_playbackWidget->setVisible(false);

    // ============================================================
    //  仪表盘 (Dashboard)
    // ============================================================

    m_dashboardWidget = new DashboardWidget(widgetParent);          // F5 仪表盘
    m_dashboardWidget->setObjectName("dashboardWidgetPanel");
    m_dashboardWidget->setVisible(false);

    // ============================================================
    //  终端增强 (Terminal Enhancement)
    // ============================================================

    m_terminalFilterBar = new TerminalFilterBar(widgetParent);      // F21 终端过滤
    m_terminalFilterBar->setObjectName("terminalFilterBarPanel");
    m_terminalFilterBar->setVisible(false);

    // ============================================================
    //  脚本录制 (Script Recorder)
    // ============================================================

    m_scriptRecorder = new ScriptRecorder(widgetParent);            // 脚本录制器
    m_scriptRecorder->setObjectName("scriptRecorderPanel");
    m_scriptRecorder->setVisible(false);

    // ============================================================
    //  委托给工厂辅助方法(PanelManagerFactory.cpp)
    // ============================================================

    createConnectionPanels(widgetParent);       ///< 连接层(BLE/CAN/MQTT/TCP/SPI/I2C/WS/USB)
    createProtocolPanels(widgetParent);         ///< 协议层(自定义协议/Modbus/Protobuf)
    createDebugPanels(widgetParent);            ///< 调试层(RTT/寄存器/信号线/流量/触发器)
    createChartExtensionPanels(widgetParent);   ///< 图表扩展(FFT/散点/直方图，依赖m_chartWidget)
    createToolPanels(widgetParent);             ///< 工具层(校验/转换/时间戳/数据包/对比)
    createSystemPanels(widgetParent);           ///< 系统层(插件/项目/设备/性能)

    // 统计：累计创建面板计数（allPanels().size() 给出实际面板总数）
    m_totalPanelsCreated = static_cast<quint64>(allPanels().size());
}
