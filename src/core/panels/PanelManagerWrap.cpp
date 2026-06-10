/**
 * @file PanelManagerWrap.cpp
 * @brief PanelManager BasePanel包装器创建与查找
 *
 * 从PanelManagerQuery.cpp拆分而来，包含:
 *   - wrapper(): 获取原始面板的BasePanel包装器指针
 *   - wrapPanels(): 创建所有BasePanel包装器，必须在createPanels()之后调用
 *
 * 映射表(panelMappings)和面板列表(allPanels)保留在PanelManagerQuery.cpp中。
 */

#include "core/panels/PanelManager.h"
#include "core/widgets/BasePanel.h"

// ---- 核心 ----
#include "serial/config/SerialConfigPanel.h"
#include "serial/data/DataStatistics.h"
#include "serial/commands/QuickCommandBar.h"
#include "protocol/view/ProtocolView.h"
#include "protocol/editor/FrameVisualEditor.h"
#include "chart/widget/ChartWidget.h"
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

// ---- 连接层 ----
#include "connection/ble/BleConfigPanel.h"
#include "connection/ble/BleGattBrowser.h"
#include "connection/can/CanConfigPanel.h"
#include "connection/can/CanBusMonitor.h"
#include "connection/mqtt/MqttConfigPanel.h"
#include "connection/mqtt/MqttSubscriptionPanel.h"
#include "connection/tcp/MultiConnectionPanel.h"
#include "connection/spi_i2c/SpiI2cConfigPanel.h"
#include "connection/ws/WsConfigPanel.h"
#include "connection/usb/UsbConfigPanel.h"
#include "connection/usb/UsbDescriptorViewer.h"

// ---- 协议层 ----
#include "protocol/editor/ProtocolSchemaEditor.h"
#include "protocol/modbus/ModbusConfigPanel.h"
#include "protocol/modbus/ModbusScanWidget.h"
#include "protocol/protobuf/SchemaViewer.h"

// ---- 调试层 ----
#include "rtt/RttConfigPanel.h"
#include "connection/spi_i2c/RegisterEditor.h"
#include "serial/signals/SignalLineWidget.h"
#include "serial/data/TrafficMonitorWidget.h"
#include "automation/TriggerListPanel.h"

// ---- 图表扩展 ----
#include "chart/fft/FftWidget.h"
#include "chart/stats/ScatterWidget.h"
#include "chart/stats/HistogramWidget.h"

// ---- 工具层 ----
#include "utils/checksum/ChecksumPanel.h"
#include "utils/converter/ConverterPanel.h"
#include "utils/timestamp/TimestampPanel.h"
#include "utils/packet/PacketBuilderPanel.h"
#include "core/widgets/DataDiffWidget.h"

// ---- 系统层 ----
#include "plugin/PluginConfigPanel.h"
#include "core/settings/ProjectWelcomeDialog.h"
#include "core/device/DeviceProfilePanel.h"
#include "utils/perf/PerformanceOverlay.h"

// ==================== BasePanel包装器 ====================

/** @brief 获取原始面板的BasePanel包装器指针，无包装器则返回nullptr */
BasePanel* PanelManager::wrapper(QWidget* rawPanel) const
{
    auto it = m_wrappers.constFind(rawPanel);
    return (it != m_wrappers.constEnd()) ? it.value() : nullptr;
}

/** @brief 创建所有BasePanel包装器，必须在createPanels()之后调用 */
void PanelManager::wrapPanels()
{
    QWidget* widgetParent = qobject_cast<QWidget*>(parent());

    auto wrap = [this, widgetParent](QWidget* panel, const QString& title) {
        if (!panel) return;
        auto* w = new BasePanel(panel, title, widgetParent);
        w->setVisible(false);
        w->setObjectName(panel->objectName() + "Wrapper");
        m_wrappers[panel] = w;
        ++m_totalPanelCreations;
        ++m_totalPanelRegisters;
    };

    for (const auto& descriptor : panelDescriptors()) {
        if (descriptor.wrapperPolicy != PanelWrapperPolicy::Wrapped) {
            continue;
        }
        wrap(descriptor.rawWidget, tr(descriptor.titleKey));
    }
}
