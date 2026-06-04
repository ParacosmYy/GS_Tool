/**
 * @file PanelManagerFactorySystem.cpp
 * @brief 面板管理器 - 调试层/图表扩展/工具层/系统层面板工厂方法
 *
 * 从 PanelManagerFactory.cpp 拆分而来，包含:
 *   - createDebugPanels()           调试层(RTT/寄存器/信号线/流量/触发器)
 *   - createChartExtensionPanels()  图表扩展(FFT/散点/直方图)
 *   - createToolPanels()            工具层(校验/转换/时间戳/数据包/对比)
 *   - createSystemPanels()          系统层(插件/项目/设备/性能)
 *
 * 连接层和协议层面板工厂见 PanelManagerFactory.cpp。
 */

#include "core/panels/PanelManager.h"

// ---- 调试层 ----
#include "rtt/RttConfigPanel.h"
#include "connection/spi_i2c/RegisterEditor.h"
#include "serial/signals/SignalLineWidget.h"
#include "serial/data/TrafficMonitorWidget.h"
#include "automation/TriggerListPanel.h"

// ---- 图表扩展(依赖ChartWidget::model()完整类型) ----
#include "chart/widget/ChartWidget.h"
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

// ==================== 调试层 ====================

/** @brief 创建调试层面板(RTT/寄存器/信号线/流量/触发器)，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createDebugPanels(QWidget* parent)
{
    m_rttConfigPanel = new RttConfigPanel(parent);                   // F6 RTT配置
    m_rttConfigPanel->setObjectName("rttConfigPanel");
    m_rttConfigPanel->setVisible(false);

    m_registerEditor = new RegisterEditor(parent);                   // F16 寄存器编辑
    m_registerEditor->setObjectName("registerEditorPanel");
    m_registerEditor->setVisible(false);

    m_signalLineWidget = new SignalLineWidget(parent);               // F9 信号线监控
    m_signalLineWidget->setObjectName("signalLineWidgetPanel");
    m_signalLineWidget->setVisible(false);

    m_trafficMonitorWidget = new TrafficMonitorWidget(parent);       // F9 流量监控
    m_trafficMonitorWidget->setObjectName("trafficMonitorWidgetPanel");
    m_trafficMonitorWidget->setVisible(false);

    m_triggerListPanel = new TriggerListPanel(parent);               // F7 触发器
    m_triggerListPanel->setObjectName("triggerListPanel");
    m_triggerListPanel->setVisible(false);
}

// ==================== 图表扩展 ====================

/** @brief 创建图表扩展面板(FFT/散点/直方图)，依赖m_chartWidget已创建，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createChartExtensionPanels(QWidget* parent)
{
    m_fftWidget = new FftWidget(m_chartWidget->model(), parent);    // FFT频谱
    m_fftWidget->setObjectName("fftWidgetPanel");
    m_fftWidget->setVisible(false);

    m_scatterWidget = new ScatterWidget(m_chartWidget->model(), parent); // 散点图
    m_scatterWidget->setObjectName("scatterWidgetPanel");
    m_scatterWidget->setVisible(false);

    m_histogramWidget = new HistogramWidget(m_chartWidget->model(), parent); // 直方图
    m_histogramWidget->setObjectName("histogramWidgetPanel");
    m_histogramWidget->setVisible(false);
}

// ==================== 工具层 ====================

/** @brief 创建工具层面板(校验/转换/时间戳/数据包/对比)，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createToolPanels(QWidget* parent)
{
    m_checksumPanel = new ChecksumPanel(parent);                     // F22 校验计算
    m_checksumPanel->setObjectName("checksumPanel");
    m_checksumPanel->setVisible(false);

    m_converterPanel = new ConverterPanel(parent);                   // F23 数据转换
    m_converterPanel->setObjectName("converterPanel");
    m_converterPanel->setVisible(false);

    m_timestampPanel = new TimestampPanel(parent);                   // F24 时间戳
    m_timestampPanel->setObjectName("timestampPanel");
    m_timestampPanel->setVisible(false);

    m_packetBuilderPanel = new PacketBuilderPanel(parent);           // F25 数据包构建
    m_packetBuilderPanel->setObjectName("packetBuilderPanel");
    m_packetBuilderPanel->setVisible(false);

    m_dataDiffPanel = new DataDiffWidget(parent);                    // 数据对比
    m_dataDiffPanel->setObjectName("dataDiffPanel");
    m_dataDiffPanel->setVisible(false);
}

// ==================== 系统层 ====================

/** @brief 创建系统层面板(插件/项目/设备/性能)，所有面板初始状态为隐藏 @param parent 面板的父widget */
void PanelManager::createSystemPanels(QWidget* parent)
{
    m_pluginConfigPanel = new PluginConfigPanel(parent);             // F11 插件系统
    m_pluginConfigPanel->setObjectName("pluginConfigPanel");
    m_pluginConfigPanel->setVisible(false);

    m_projectWelcomeDialog = new ProjectWelcomeDialog(parent);      // F8 项目管理
    m_projectWelcomeDialog->setObjectName("projectWelcomeDialog");
    m_projectWelcomeDialog->setVisible(false);

    m_deviceProfilePanel = new DeviceProfilePanel(parent);           // F26 设备档案
    m_deviceProfilePanel->setObjectName("deviceProfilePanel");
    m_deviceProfilePanel->setVisible(false);

    m_performanceOverlay = new PerformanceOverlay(parent);           // F10 性能监控
    m_performanceOverlay->setObjectName("performanceOverlay");
    m_performanceOverlay->setVisible(false);
}
