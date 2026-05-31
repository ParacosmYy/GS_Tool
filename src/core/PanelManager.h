#ifndef PANEL_MANAGER_H
#define PANEL_MANAGER_H

#include <QObject>
#include <QVector>
#include "core/NavigationController.h"

class SerialConfigPanel;
class DataStatistics;
class ProtocolView;
class FrameVisualEditor;
class ChartWidget;
class OtaWidget;
class OtaManager;
class TerminalWidget;
class TerminalModel;
class TerminalSearchBar;
class QuickCommandBar;
class BookmarkWidget;

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
class PanelManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造面板管理器
     * @param parent 父对象，面板 widget 的 QObject 父树根节点
     */
    explicit PanelManager(QObject* parent = nullptr);

    /** @brief 析构，QObject 父子树自动销毁所有面板 widget */
    ~PanelManager() override;

    // 禁止拷贝和赋值（QObject 派生类）
    PanelManager(const PanelManager&) = delete;
    PanelManager& operator=(const PanelManager&) = delete;

    /**
     * @brief 创建所有面板 widget（调用一次，在 MainWindow::setupUI 中）
     *
     * @param otaManager OTA 管理器指针，OtaWidget 构造时需要注入此依赖
     * @param terminalModel 终端数据模型指针，TerminalWidget 需要设置此模型
     *
     * 面板创建后初始状态均为隐藏，由 NavigationController 按需显示。
     * 调用此方法后即可通过 Getter 获取各面板指针。
     */
    void createPanels(OtaManager* otaManager, TerminalModel* terminalModel);

    // ==================== 面板 Getter ====================

    /** @brief 获取串口配置面板（端口/波特率/数据位/校验/流控参数选择） */
    SerialConfigPanel* serialConfig() const;

    /** @brief 获取数据统计面板（RX/TX 累计字节数和速率显示） */
    DataStatistics* dataStats() const;

    /** @brief 获取协议解析视图（表格形式展示解析后的帧数据） */
    ProtocolView* protocolView() const;

    /** @brief 获取帧可视化编辑器（GUI 界面定义帧结构） */
    FrameVisualEditor* frameEditor() const;

    /** @brief 获取波形图控件（实时绘制解析后的数值数据） */
    ChartWidget* chartWidget() const;

    /** @brief 获取 OTA 升级面板（文件选择/协议选择/进度显示） */
    OtaWidget* otaWidget() const;

    /** @brief 获取终端显示控件（自绘引擎支持搜索高亮/HEX/文本/十进制显示） */
    TerminalWidget* terminal() const;

    /** @brief 获取终端搜索栏（正则/HEX 搜索和匹配计数显示） */
    TerminalSearchBar* searchBar() const;

    /** @brief 获取快捷指令栏（预置常用 AT 命令和自定义指令） */
    QuickCommandBar* quickCmdBar() const;

    /** @brief 获取书签面板（展示和管理录制时间轴上的书签标记） */
    BookmarkWidget* bookmarkWidget() const;

    // ==================== 映射表接口 ====================

    /**
     * @brief 获取面板映射表
     * 传给 NavigationController::buildNavTree() 用于构建导航树数据模型
     * @return 面板名称到 QWidget 的映射表
     */
    QVector<NavPanelMapping> panelMappings() const;

    /**
     * @brief 获取所有可切换的面板列表
     * 用于 NavigationController 切换面板时批量隐藏所有面板
     * @return 所有面板 widget 列表
     */
    QVector<QWidget*> allPanels() const;

private:
    // ==================== 面板指针 ====================

    /** @brief 串口配置面板 */
    SerialConfigPanel* m_serialConfig = nullptr;

    /** @brief 数据统计面板 */
    DataStatistics* m_dataStats = nullptr;

    /** @brief 协议解析视图 */
    ProtocolView* m_protocolView = nullptr;

    /** @brief 帧可视化编辑器 */
    FrameVisualEditor* m_frameEditor = nullptr;

    /** @brief 波形图控件 */
    ChartWidget* m_chartWidget = nullptr;

    /** @brief OTA 升级面板 */
    OtaWidget* m_otaWidget = nullptr;

    /** @brief 终端显示控件 */
    TerminalWidget* m_terminal = nullptr;

    /** @brief 终端搜索栏 */
    TerminalSearchBar* m_searchBar = nullptr;

    /** @brief 快捷指令栏 */
    QuickCommandBar* m_quickCmdBar = nullptr;

    /** @brief 书签面板 */
    BookmarkWidget* m_bookmarkWidget = nullptr;
};

#endif // PANEL_MANAGER_H
