/**
 * @file NavigationController.cpp
 * @brief 导航控制器实现 - 导航树构建、面板切换滑动动画、呼吸动画
 *
 * 面板切换动画流程 (CLAUDE.md 6.5 滑入滑出规范):
 *   1. 防重入检查（动画进行中忽略）
 *   2. 隐藏所有非目标、非旧面板（清除残留 opacity effect）
 *   3. 计算滑动宽度（面板父容器宽度）
 *   4. 并行执行:
 *      - 旧面板: pos (0,0)->(-width,0) 200ms InCubic + opacity 1->0
 *      - 新面板: pos (width,0)->(0,0) 250ms OutCubic + opacity 0->1
 *   5. 动画期间禁用导航树，完成后恢复
 *   6. 动画对象使用 DeleteWhenStopped 自动清理
 */

#include "core/NavigationController.h"
#include "core/Constants.h"
#include "core/ThemeManager.h"
#include <QTreeView>
#include <QStandardItemModel>
#include <QPainter>
#include <QPixmap>
#include <QLabel>
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

/** @brief 获取面板父容器宽度（serialPanel）作为滑动距离 */
int NavigationController::parentContainerWidth(QWidget* panel) const
{
    QWidget* parent = panel ? panel->parentWidget() : nullptr;
    return parent ? parent->width() : 0;
}

/** @brief 旧面板滑出+淡出: pos (0,0)->(-width,0) InCubic, opacity 1->0 */
void NavigationController::animateSlideOut(QWidget* oldPanel, QParallelAnimationGroup* group)
{
    const int slideWidth = parentContainerWidth(oldPanel);
    if (slideWidth <= 0) { oldPanel->setVisible(false); return; } ///< 容器宽度无效时跳过

    QGraphicsOpacityEffect* fadeOutEffect = new QGraphicsOpacityEffect(oldPanel);
    fadeOutEffect->setOpacity(1.0);
    oldPanel->setGraphicsEffect(fadeOutEffect);

    QPropertyAnimation* slideOut = new QPropertyAnimation(oldPanel, "pos");
    slideOut->setStartValue(QPoint(0, 0));
    slideOut->setEndValue(QPoint(-slideWidth, 0));
    slideOut->setDuration(Animations::kPanelSlideOutMs);
    slideOut->setEasingCurve(QEasingCurve::InCubic);

    QPropertyAnimation* fadeOut = new QPropertyAnimation(fadeOutEffect, "opacity");
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setDuration(Animations::kPanelSlideOutMs);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    // 动画组完成后再清除effect + 隐藏 + 恢复位置
    // 注意: 不能连接slideOut单独的finished，因为fadeOut可能还在使用fadeOutEffect
    connect(group, &QParallelAnimationGroup::finished, this, [oldPanel]() {
        if (oldPanel->graphicsEffect()) oldPanel->setGraphicsEffect(nullptr);
        oldPanel->setVisible(false);
        oldPanel->move(0, 0);
    });

    group->addAnimation(slideOut);
    group->addAnimation(fadeOut);
}

/** @brief 新面板滑入+淡入: pos (width,0)->(0,0) OutCubic, opacity 0->1 */
void NavigationController::animateSlideIn(QWidget* newPanel, QParallelAnimationGroup* group)
{
    const int slideWidth = parentContainerWidth(newPanel);
    if (slideWidth <= 0) {  ///< 容器宽度无效时跳过动画
        newPanel->setGraphicsEffect(nullptr);
        newPanel->move(0, 0);
        newPanel->setVisible(true);
        return;
    }

    QGraphicsOpacityEffect* fadeInEffect = new QGraphicsOpacityEffect(newPanel);
    fadeInEffect->setOpacity(0.0);
    newPanel->setGraphicsEffect(fadeInEffect);
    newPanel->move(slideWidth, 0);  ///< 放置到右侧屏幕外
    newPanel->setVisible(true);
    newPanel->raise();              ///< 提升到最前（覆盖旧面板）

    QPropertyAnimation* slideIn = new QPropertyAnimation(newPanel, "pos");
    slideIn->setStartValue(QPoint(slideWidth, 0));
    slideIn->setEndValue(QPoint(0, 0));
    slideIn->setDuration(Animations::kPanelSlideInMs);
    slideIn->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation* fadeIn = new QPropertyAnimation(fadeInEffect, "opacity");
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setDuration(Animations::kPanelSlideInMs);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    // 滑入完成: 清除 effect 恢复正常绘制性能
    connect(slideIn, &QPropertyAnimation::finished, this, [newPanel, fadeInEffect]() {
        if (newPanel->graphicsEffect() == fadeInEffect) newPanel->setGraphicsEffect(nullptr);
    });

    group->addAnimation(slideIn);
    group->addAnimation(fadeIn);
}

/**
 * @brief 面板切换（带滑入滑出动画）
 *
 * 流程 (CLAUDE.md 6.5): 防重入 -> 清理残留 -> 创建并行动画组
 * 旧面板: pos (0,0)->(-width,0) 200ms InCubic + opacity 1->0
 * 新面板: pos (width,0)->(0,0) 250ms OutCubic + opacity 0->1
 * 动画期间禁用导航树，完成后恢复。动画组使用 deleteLater 自动清理。
 *
 * @param newPanel 目标面板 widget
 */
void NavigationController::switchToPanel(QWidget* newPanel)
{
    if (m_panelSwitching) return;           ///< 防重入: 动画进行中忽略
    if (m_currentPanel == newPanel) return; ///< 目标与当前相同，无需切换

    QWidget* oldPanel = m_currentPanel;
    m_currentPanel = newPanel;

    // 隐藏所有非当前、非旧面板，并清除残留的 opacity effect 和位移偏移
    for (auto* w : allSwitchablePanels()) {
        if (w && w != newPanel && w != oldPanel) {
            if (w->graphicsEffect()) w->setGraphicsEffect(nullptr);
            w->move(0, 0);
            w->setVisible(false);
        }
    }

    if (!newPanel) return;

    m_panelSwitching = true;
    if (m_navTree) m_navTree->setDisabled(true);  ///< 动画期间禁用导航树

    // 创建并行动画组: 旧面板滑出(如有)和新面板滑入同时执行
    auto* animGroup = new QParallelAnimationGroup(this);
    m_switchAnimGroup = animGroup;

    if (oldPanel && oldPanel->isVisible()) {
        animateSlideOut(oldPanel, animGroup);  ///< 旧面板: 滑出+淡出 200ms InCubic
    }
    animateSlideIn(newPanel, animGroup);       ///< 新面板: 滑入+淡入 250ms OutCubic

    // 动画完成: 恢复导航树交互 + 重置切换标志 + 自动清理动画组
    connect(animGroup, &QParallelAnimationGroup::finished, this, [this]() {
        m_panelSwitching = false;
        m_switchAnimGroup = nullptr;
        if (m_navTree) m_navTree->setDisabled(false);
    });
    connect(animGroup, &QParallelAnimationGroup::finished,
            animGroup, &QObject::deleteLater);

    animGroup->start();
}

/**
 * @brief 启动连接状态呼吸动画
 *
 * 使用 QSequentialAnimationGroup 实现平滑往返脉冲:
 *   上半周期: opacity 0.3 -> 1.0, 1500ms, InOutSine (淡入)
 *   下半周期: opacity 1.0 -> 0.3, 1500ms, InOutSine (淡出)
 *   无限循环, 避免单方向动画结束时从1.0跳变到0.3的突兀感
 *
 * @param statusLabel 状态标签控件
 */
void NavigationController::startBreathingAnimation(QLabel* statusLabel)
{
    // 如果已有呼吸动画在运行，不重复创建
    if (m_breathingAnim && m_breathingAnim->state() == QAbstractAnimation::Running) {
        return;
    }

    // 为状态标签创建透明度效果
    if (!m_connStatusEffect) {
        m_connStatusEffect = new QGraphicsOpacityEffect(statusLabel);
        statusLabel->setGraphicsEffect(m_connStatusEffect);
    }
    m_connStatusEffect->setOpacity(1.0);

    // 销毁旧动画(如果存在)
    // 旧动画使用 DeleteWhenStopped + 无限循环，stop() 触发自动销毁
    if (m_breathingAnim) {
        m_breathingAnim->stop();
        m_breathingAnim = nullptr;
    }

    // 构建呼吸动画: 顺序组 [0.3->1.0, 1500ms] + [1.0->0.3, 1500ms], 无限循环
    auto* group = new QSequentialAnimationGroup(this);

    // 上半周期: 0.3 -> 1.0 (淡入)
    auto* fadeIn = new QPropertyAnimation(m_connStatusEffect, "opacity");
    fadeIn->setStartValue(0.3);
    fadeIn->setEndValue(1.0);
    fadeIn->setDuration(Animations::kBreatheCycleMs);
    fadeIn->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(fadeIn);

    // 下半周期: 1.0 -> 0.3 (淡出)
    auto* fadeOut = new QPropertyAnimation(m_connStatusEffect, "opacity");
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.3);
    fadeOut->setDuration(Animations::kBreatheCycleMs);
    fadeOut->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(fadeOut);

    group->setLoopCount(-1);  // 无限循环
    group->start(QAbstractAnimation::DeleteWhenStopped);
    m_breathingAnim = group;
}

/**
 * @brief 停止连接状态呼吸动画
 *
 * 停止动画组 -> 恢复标签完全不透明 -> 清理 effect 对象。
 * 动画组使用 DeleteWhenStopped，stop() 后 Qt 自动销毁，此处仅清空指针。
 *
 * @param statusLabel 状态标签控件（析构时可传 nullptr）
 */
void NavigationController::stopBreathingAnimation(QLabel* statusLabel)
{
    if (m_breathingAnim) {
        // DeleteWhenStopped: stop() 后 Qt 自动 delete，不可再次 delete
        m_breathingAnim->stop();
        m_breathingAnim = nullptr;
    }
    // 恢复状态标签完全不透明
    // 注意: setGraphicsEffect(nullptr) 会自动 delete 旧的 effect，不可手动再 delete
    if (m_connStatusEffect) {
        m_connStatusEffect->setOpacity(1.0);
        if (statusLabel) {
            statusLabel->setGraphicsEffect(nullptr);  // Qt 自动 delete m_connStatusEffect
        }
        m_connStatusEffect = nullptr;
    }
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
