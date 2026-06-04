/**
 * @file PluginConfigPanel.cpp
 * @brief 插件配置面板实现 — 构造函数与管理器绑定
 *
 * 面板布局: 端口选择 → 参数配置 → 控制信号 → 驱动检测 → 连接按钮+状态指示器
 *
 * UI构建和信号连接见 PluginConfigPanelSetup.cpp。
 * 列表刷新和详情面板更新见 PluginConfigPanelUI.cpp。
 */

#include "plugin/PluginConfigPanel.h"
#include "plugin/PluginManager.h"

/**
 * @brief 构造函数
 *
 * 初始化 UI 布局，创建控件并连接信号。
 *
 * @param parent 父控件
 */
PluginConfigPanel::PluginConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_pluginList(nullptr)
    , m_loadBtn(nullptr)
    , m_unloadBtn(nullptr)
    , m_scanBtn(nullptr)
    , m_detailNameLabel(nullptr)
    , m_detailVersionLabel(nullptr)
    , m_detailDescEdit(nullptr)
    , m_detailStatusLabel(nullptr)
    , m_manager(nullptr)
{
    setObjectName(QStringLiteral("PluginConfigPanel"));
    setupUI();
}

/**
 * @brief 绑定插件管理器
 *
 * 保存指针并连接插件加载/卸载信号，以便自动刷新列表。
 *
 * @param manager PluginManager 实例
 */
void PluginConfigPanel::setPluginManager(PluginManager* manager)
{
    if (m_manager) {
        /* 断开旧连接 */
        disconnect(m_manager, &PluginManager::pluginLoaded,
                   this, nullptr);
        disconnect(m_manager, &PluginManager::pluginUnloaded,
                   this, nullptr);
    }

    m_manager = manager;
    ++m_stats.totalConfigChanges;

    if (m_manager) {
        connect(m_manager, &PluginManager::pluginLoaded,
                this, [this]() { refreshList(); });
        connect(m_manager, &PluginManager::pluginUnloaded,
                this, [this]() { refreshList(); });
        refreshList();
    }
}

// setupUI/信号连接见 PluginConfigPanelSetup.cpp
// refreshList/updateDetailPanel见 PluginConfigPanelUI.cpp
