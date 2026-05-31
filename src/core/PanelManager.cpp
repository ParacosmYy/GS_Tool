/**
 * @file PanelManager.cpp
 * @brief 面板管理器实现 - 统一创建和管理所有功能面板 widget
 *
 * 本文件实现 PanelManager 的面板创建、Getter 和映射表接口。
 * 所有面板 widget 通过 QObject 父子树管理生命周期，PanelManager 作为父对象。
 */

#include "core/PanelManager.h"
#include "serial/SerialConfigPanel.h"
#include "serial/DataStatistics.h"
#include "serial/QuickCommandBar.h"
#include "protocol/ProtocolView.h"
#include "protocol/FrameVisualEditor.h"
#include "chart/ChartWidget.h"
#include "ota/OtaManager.h"
#include "ota/OtaWidget.h"
#include "terminal/TerminalWidget.h"
#include "terminal/TerminalModel.h"
#include "terminal/TerminalSearchBar.h"
#include "serial/BookmarkWidget.h"

/** @brief 构造面板管理器，初始化所有面板指针为空 */
PanelManager::PanelManager(QObject* parent)
    : QObject(parent)
{
}

/** @brief 析构 - QObject 父子树自动销毁所有面板 widget */
PanelManager::~PanelManager()
{
}

/**
 * @brief 创建所有面板 widget
 *
 * @param otaManager OTA 管理器指针，OtaWidget 构造时需要注入此依赖
 * @param terminalModel 终端数据模型指针，TerminalWidget 需要设置此模型
 *
 * 面板创建顺序:
 * 1. SerialConfigPanel - 串口配置面板
 * 2. DataStatistics - 数据统计面板
 * 3. ProtocolView - 协议解析视图
 * 4. FrameVisualEditor - 帧可视化编辑器
 * 5. ChartWidget - 波形图控件
 * 6. OtaWidget - OTA 升级面板
 * 7. TerminalWidget - 终端显示控件
 * 8. TerminalSearchBar - 终端搜索栏
 * 9. QuickCommandBar - 快捷指令栏
 *
 * 所有面板初始状态为隐藏，由 NavigationController 按需显示。
 */
void PanelManager::createPanels(OtaManager* otaManager, TerminalModel* terminalModel)
{
    // ---- 串口配置面板: 端口/波特率/数据位/校验/流控参数选择 ----
    // 注意: 此处不传 QWidget* parent，因为 PanelManager 是 QObject 非 QWidget。
    // 面板被添加到 MainWindow 布局时由 Qt 自动 reparent，生命周期安全。
    m_serialConfig = new SerialConfigPanel();
    m_serialConfig->setObjectName("serialConfigPanel");
    m_serialConfig->setVisible(false);

    // ---- 数据统计面板: RX/TX 累计字节数和速率显示 ----
    m_dataStats = new DataStatistics();
    m_dataStats->setObjectName("dataStatsPanel");
    m_dataStats->setVisible(false);

    // ---- 协议解析视图: 以表格形式展示解析后的帧数据 ----
    m_protocolView = new ProtocolView();
    m_protocolView->setObjectName("protocolViewPanel");
    m_protocolView->setVisible(false);

    // ---- 帧可视化编辑器: GUI 界面定义帧结构（帧头/字段/CRC） ----
    m_frameEditor = new FrameVisualEditor();
    m_frameEditor->setObjectName("frameEditorPanel");
    m_frameEditor->setVisible(false);

    // ---- 波形图控件: 实时绘制解析后的数值数据，支持滑动窗口和降采样 ----
    m_chartWidget = new ChartWidget();
    m_chartWidget->setObjectName("chartWidgetPanel");
    m_chartWidget->setVisible(false);

    // ---- OTA 升级面板: 文件选择、协议选择、进度显示和错误反馈 ----
    m_otaWidget = new OtaWidget(otaManager);
    m_otaWidget->setObjectName("otaWidgetPanel");
    m_otaWidget->setVisible(false);

    // ---- 终端显示控件: 自绘引擎支持搜索高亮、HEX/文本/十进制显示 ----
    m_terminal = new TerminalWidget();
    m_terminal->setObjectName("terminalPanel");
    m_terminal->setModel(terminalModel);

    // ---- 终端搜索栏: 支持正则/HEX 搜索和匹配计数显示 ----
    m_searchBar = new TerminalSearchBar();
    m_searchBar->setObjectName("searchBarPanel");

    // ---- 快捷指令栏: 预置常用 AT 命令和自定义指令的一键发送 ----
    m_quickCmdBar = new QuickCommandBar();
    m_quickCmdBar->setCommands({
        {"AT", "AT\r\n", false},
        {"Reset", "AA 55 01 00 FE", true},
        {"Status", "AT+STATUS?\r\n", false}
    });

    // ---- 书签面板: 展示和管理录制时间轴上的书签标记 ----
    // 支持双击跳转、添加/删除/清空书签操作，与 DataLogger 联动
    m_bookmarkWidget = new BookmarkWidget();
    m_bookmarkWidget->setObjectName("bookmarkWidgetPanel");
    m_bookmarkWidget->setVisible(false);
}

// ==================== Getter 实现 ====================

/** @brief 获取串口配置面板 */
SerialConfigPanel* PanelManager::serialConfig() const { return m_serialConfig; }

/** @brief 获取数据统计面板 */
DataStatistics* PanelManager::dataStats() const { return m_dataStats; }

/** @brief 获取协议解析视图 */
ProtocolView* PanelManager::protocolView() const { return m_protocolView; }

/** @brief 获取帧可视化编辑器 */
FrameVisualEditor* PanelManager::frameEditor() const { return m_frameEditor; }

/** @brief 获取波形图控件 */
ChartWidget* PanelManager::chartWidget() const { return m_chartWidget; }

/** @brief 获取 OTA 升级面板 */
OtaWidget* PanelManager::otaWidget() const { return m_otaWidget; }

/** @brief 获取终端显示控件 */
TerminalWidget* PanelManager::terminal() const { return m_terminal; }

/** @brief 获取终端搜索栏 */
TerminalSearchBar* PanelManager::searchBar() const { return m_searchBar; }

/** @brief 获取快捷指令栏 */
QuickCommandBar* PanelManager::quickCmdBar() const { return m_quickCmdBar; }

/** @brief 获取书签面板 */
BookmarkWidget* PanelManager::bookmarkWidget() const { return m_bookmarkWidget; }

/**
 * @brief 获取面板映射表
 *
 * 使用 QT_TRANSLATE_NOOP 标记翻译键，运行时通过 tr() 翻译。
 * 映射表顺序对应导航树的叶子节点顺序。
 */
QVector<NavPanelMapping> PanelManager::panelMappings() const
{
    return {
        {QT_TRANSLATE_NOOP("MainWindow", "配置"),     m_serialConfig},
        {QT_TRANSLATE_NOOP("MainWindow", "终端"),     m_terminal},
        {QT_TRANSLATE_NOOP("MainWindow", "统计"),     m_dataStats},
        {QT_TRANSLATE_NOOP("MainWindow", "协议"),     m_protocolView},
        {QT_TRANSLATE_NOOP("MainWindow", "帧编辑器"), m_frameEditor},
        {QT_TRANSLATE_NOOP("MainWindow", "波形图"),   m_chartWidget},
        {QT_TRANSLATE_NOOP("MainWindow", "OTA升级"),  m_otaWidget},
        {QT_TRANSLATE_NOOP("MainWindow", "书签"),     m_bookmarkWidget},
    };
}

/**
 * @brief 获取所有可切换的面板列表
 *
 * 包含所有可通过导航树切换显示的面板 widget，
 * 用于 NavigationController::switchToPanel() 中先隐藏所有面板再显示目标面板。
 */
QVector<QWidget*> PanelManager::allPanels() const
{
    return {
        m_serialConfig,
        m_terminal,
        m_dataStats,
        m_protocolView,
        m_frameEditor,
        m_chartWidget,
        m_otaWidget,
        m_bookmarkWidget,
    };
}
