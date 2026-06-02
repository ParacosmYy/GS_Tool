/**
 * @file PluginConfigPanel.cpp
 * @brief 插件配置面板实现 — 骨架文件
 */

#include "plugin/PluginConfigPanel.h"
#include "plugin/PluginManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

/**
 * @brief 构造函数
 * @param parent 父控件
 */
PluginConfigPanel::PluginConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_pluginList(nullptr)
    , m_loadBtn(nullptr)
    , m_unloadBtn(nullptr)
    , m_manager(nullptr)
{
    setObjectName(QStringLiteral("PluginConfigPanel"));
    setupUI();
}

/**
 * @brief 绑定插件管理器
 *
 * 连接管理器的信号以刷新列表显示。
 *
 * @param manager PluginManager 实例
 */
void PluginConfigPanel::setPluginManager(PluginManager* manager)
{
    Q_UNUSED(manager)
    // TODO: 保存指针，连接信号，刷新列表
}

/**
 * @brief 初始化 UI 布局和控件
 *
 * 左侧插件列表，右侧加载/卸载按钮垂直排列。
 */
void PluginConfigPanel::setupUI()
{
    // TODO: 创建并布局所有 UI 控件
    // m_pluginList = new QListWidget(this);
    // m_loadBtn = new QPushButton(tr("加载"), this);
    // m_unloadBtn = new QPushButton(tr("卸载"), this);
}
