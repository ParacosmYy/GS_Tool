/**
 * @file NavigationController.cpp
 * @brief 导航控制器实现 - 导航树构建、面板查找、面板状态管理
 *
 * 动画相关方法已拆分至 NavigationControllerAnimations.cpp:
 *   - animateSlideOut / animateSlideIn / switchToPanel (面板滑动切换)
 *   - startBreathingAnimation / stopBreathingAnimation (连接状态呼吸动画)
 *   - parentContainerWidth (动画辅助)
 */

#include "core/NavigationController.h"
#include "core/ThemeManager.h"
#include <QTreeView>
#include <QStandardItemModel>
#include <QPainter>
#include <QPixmap>
#include <QCoreApplication>

/**
 * @brief 创建导航树连接类型指示圆点图标
 * 8x8 透明底 + 抗锯齿彩色圆点，用于区分不同连接类型
 * @param color 圆点颜色
 * @return 图标实例
 */
static QIcon createDotIcon(const QColor& color)
{
    QPixmap dot(8, 8);
    dot.fill(Qt::transparent);
    QPainter painter(&dot);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(1, 1, 6, 6);
    return QIcon(dot);
}

/** @brief 构造导航控制器 */
NavigationController::NavigationController(QObject* parent)
    : QObject(parent)
{
    // 主题切换时刷新导航树图标颜色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &NavigationController::onThemeChanged);
}

/**
 * @brief 析构导航控制器
 * 清理呼吸动画和进行中的切换动画，防止资源泄漏
 */
NavigationController::~NavigationController()
{
    // 清理进行中的面板切换动画
    if (m_switchAnimGroup) {
        m_switchAnimGroup->stop();
        // QParallelAnimationGroup 的子动画如果使用 DeleteWhenStopped
        // 在 group stop 时会被自动删除，这里只需删除 group 自身
        delete m_switchAnimGroup;
        m_switchAnimGroup = nullptr;
    }
    stopBreathingAnimation(nullptr);
}

/**
 * @brief 构建导航树模型并展开全部节点
 *
 * 导航树结构:
 *   串口 (蓝点) --+-- 配置 / 终端 / 统计 / 协议 / 帧编辑器 / 波形图 / OTA升级
 *   网络 --+-- TCP客户端(绿点) / TCP服务端(绿点) / UDP(黄点)
 *   工具 --- 数据导出
 *
 * @param navTree 导航树视图控件
 * @param mappings 面板名称到 QWidget 的映射表
 */
void NavigationController::buildNavTree(QTreeView* navTree, const QVector<NavPanelMapping>& mappings)
{
    m_navTree = navTree;
    m_navPanelMappings = mappings;

    auto* treeModel = new QStandardItemModel(this);
    auto* rootItem = treeModel->invisibleRootItem();

    // 串口分组 -- 蓝色圆点标识（从ThemeManager获取Accent色）
    auto* serialItem = new QStandardItem(createDotIcon(
        ThemeManager::instance().color(ThemeManager::SemanticColor::Accent)), tr("串口"));
    serialItem->setEditable(false);
    auto* configItem = new QStandardItem(tr("配置"));
    configItem->setEditable(false);
    auto* terminalItem = new QStandardItem(tr("终端"));
    terminalItem->setEditable(false);
    auto* statsItem = new QStandardItem(tr("统计"));
    statsItem->setEditable(false);
    serialItem->appendRow(configItem);
    serialItem->appendRow(terminalItem);
    serialItem->appendRow(statsItem);
    auto* protocolItem = new QStandardItem(tr("协议"));
    protocolItem->setEditable(false);
    auto* frameEditorItem = new QStandardItem(tr("帧编辑器"));
    frameEditorItem->setEditable(false);
    auto* chartItem = new QStandardItem(tr("波形图"));
    chartItem->setEditable(false);
    serialItem->appendRow(protocolItem);
    serialItem->appendRow(frameEditorItem);
    serialItem->appendRow(chartItem);
    auto* otaItem = new QStandardItem(tr("OTA升级"));
    otaItem->setEditable(false);
    serialItem->appendRow(otaItem);
    auto* bookmarkItem = new QStandardItem(tr("书签"));
    bookmarkItem->setEditable(false);
    serialItem->appendRow(bookmarkItem);

    // 网络分组 -- TCP 绿色圆点, UDP 黄色圆点（从ThemeManager获取语义色）
    auto* networkItem = new QStandardItem(tr("网络"));
    networkItem->setEditable(false);
    auto* tcpClientItem = new QStandardItem(createDotIcon(
        ThemeManager::instance().color(ThemeManager::SemanticColor::Success)), tr("TCP客户端"));
    tcpClientItem->setEditable(false);
    auto* tcpServerItem = new QStandardItem(createDotIcon(
        ThemeManager::instance().color(ThemeManager::SemanticColor::Success)), tr("TCP服务端"));
    tcpServerItem->setEditable(false);
    auto* udpItem = new QStandardItem(createDotIcon(
        ThemeManager::instance().color(ThemeManager::SemanticColor::Warning)), tr("UDP"));
    udpItem->setEditable(false);
    networkItem->appendRow(tcpClientItem);
    networkItem->appendRow(tcpServerItem);
    networkItem->appendRow(udpItem);

    // 工具分组
    auto* toolsItem = new QStandardItem(tr("工具"));
    toolsItem->setEditable(false);
    auto* exportItem = new QStandardItem(tr("数据导出"));
    exportItem->setEditable(false);
    toolsItem->appendRow(exportItem);

    rootItem->appendRow(serialItem);
    rootItem->appendRow(networkItem);
    rootItem->appendRow(toolsItem);

    navTree->setModel(treeModel);
    navTree->expandAll();  // 默认展开所有分组
}

/** @brief 收集所有可切换面板 widget（从映射表中提取所有非空 widget） */
QVector<QWidget*> NavigationController::allSwitchablePanels() const
{
    QVector<QWidget*> panels;
    panels.reserve(m_navPanelMappings.size());
    for (const auto& mapping : m_navPanelMappings) {
        if (mapping.widget) {
            panels.append(mapping.widget);
        }
    }
    return panels;
}

/** @brief 获取当前面板在映射表中的索引，用于持久化保存 @return 0~N-1, 未找到返回-1 */
int NavigationController::currentPanelIndex() const
{
    if (!m_currentPanel) return -1;

    for (int i = 0; i < m_navPanelMappings.size(); ++i) {
        if (m_navPanelMappings[i].widget == m_currentPanel) {
            return i;
        }
    }
    return -1;
}

/** @brief 通过索引恢复面板（启动时使用，不触发动画） @return true成功 false越界/空 */
bool NavigationController::restorePanelByIndex(int index)
{
    if (index < 0 || index >= m_navPanelMappings.size()) {
        return false;
    }

    QWidget* target = m_navPanelMappings[index].widget;
    if (!target) return false;

    for (auto* w : allSwitchablePanels()) {
        if (w && w != target) {
            if (w->graphicsEffect()) w->setGraphicsEffect(nullptr);
            w->move(0, 0);
            w->setVisible(false);
        }
    }

    // 确保目标面板位置正确且无残留 effect
    target->setGraphicsEffect(nullptr);
    target->move(0, 0);
    target->setVisible(true);
    m_currentPanel = target;

    return true;
}

/** @brief 设置当前面板（初始化用，不触发动画） */
void NavigationController::setCurrentPanel(QWidget* panel)
{
    m_currentPanel = panel;
}

/** @brief 通过翻译后的名称查找对应的 panel widget，未找到返回 nullptr */
QWidget* NavigationController::lookupPanel(const QString& translatedName) const
{
    for (const auto& mapping : m_navPanelMappings) {
        if (translatedName == QCoreApplication::translate("MainWindow", mapping.name)) {
            return mapping.widget;
        }
    }
    return nullptr;
}

/**
 * @brief 主题切换时刷新导航树圆点图标颜色
 *
 * buildNavTree()中的圆点图标在构建时读取ThemeManager颜色，
 * 主题切换后需要重新着色以匹配新主题。
 * 导航树结构: root -> 串口(0, Accent蓝) / 网络(1) -> TCP客户端(0,Success绿)
 *                                            -> TCP服务端(1,Success绿)
 *                                            -> UDP(2,Warning黄)
 */
void NavigationController::onThemeChanged()
{
    if (!m_navTree) return;

    auto* model = qobject_cast<QStandardItemModel*>(m_navTree->model());
    if (!model) return;

    auto* root = model->invisibleRootItem();
    if (!root) return;

    // 串口分组(第0行) — 蓝色圆点(Accent)
    auto* serialItem = root->child(0);
    if (serialItem) {
        serialItem->setIcon(createDotIcon(
            ThemeManager::instance().color(ThemeManager::SemanticColor::Accent)));
    }

    // 网络分组(第1行) — 子项有TCP(绿/Success)和UDP(黄/Warning)
    auto* networkItem = root->child(1);
    if (networkItem) {
        // TCP客户端(第0行)
        auto* tcpClient = networkItem->child(0);
        if (tcpClient) {
            tcpClient->setIcon(createDotIcon(
                ThemeManager::instance().color(ThemeManager::SemanticColor::Success)));
        }
        // TCP服务端(第1行)
        auto* tcpServer = networkItem->child(1);
        if (tcpServer) {
            tcpServer->setIcon(createDotIcon(
                ThemeManager::instance().color(ThemeManager::SemanticColor::Success)));
        }
        // UDP(第2行)
        auto* udpItem = networkItem->child(2);
        if (udpItem) {
            udpItem->setIcon(createDotIcon(
                ThemeManager::instance().color(ThemeManager::SemanticColor::Warning)));
        }
    }
}
