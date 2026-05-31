/**
 * @file NavigationController.cpp
 * @brief 导航控制器实现 - 导航树构建、面板切换动画、呼吸动画
 *
 * 面板切换动画流程:
 *   淡出旧面板(150ms InCubic) → 隐藏旧面板 → 显示新面板 → 淡入新面板(150ms OutCubic)
 * 动画期间通过 m_panelSwitching 标志防止重复触发。
 */

#include "NavigationController.h"
#include "Constants.h"
#include "ThemeManager.h"
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
}

/**
 * @brief 析构导航控制器
 * 清理呼吸动画和透明度效果，防止资源泄漏
 */
NavigationController::~NavigationController()
{
    stopBreathingAnimation(nullptr);
}

/**
 * @brief 构建导航树模型并展开全部节点
 *
 * 导航树结构:
 *   串口 (蓝点) ─┬─ 配置 / 终端 / 统计 / 协议 / 帧编辑器 / 波形图 / OTA升级
 *   网络 ─┬─ TCP客户端(绿点) / TCP服务端(绿点) / UDP(黄点)
 *   工具 ─── 数据导出
 *
 * @param navTree 导航树视图控件
 * @param mappings 面板名称到 QWidget 的映射表
 */
void NavigationController::buildNavTree(QTreeView* navTree, const QVector<NavPanelMapping>& mappings)
{
    m_navPanelMappings = mappings;

    auto* treeModel = new QStandardItemModel(this);
    auto* rootItem = treeModel->invisibleRootItem();

    // 串口分组 — 蓝色圆点标识（从ThemeManager获取Accent色）
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

    // 网络分组 — TCP 绿色圆点, UDP 黄色圆点（从ThemeManager获取语义色）
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

/**
 * @brief 收集所有可切换面板 widget
 * @return 面板 widget 列表（从映射表中提取所有非空 widget）
 */
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

/**
 * @brief 获取当前面板在映射表中的索引
 *
 * 遍历映射表查找 m_currentPanel 对应的位置，
 * 用于在关闭窗口时保存用户上次查看的面板。
 *
 * @return 面板索引（0~N-1），未找到时返回 -1
 */
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

/**
 * @brief 通过索引恢复面板显示（启动时使用，不触发动画）
 *
 * 与 switchToPanel() 不同，此方法:
 *   - 不执行淡入淡出动画（启动时不需要过渡效果）
 *   - 直接隐藏所有面板后显示目标面板
 *   - 更新 m_currentPanel 为目标面板
 *
 * @param index 面板索引
 * @return true 恢复成功，false 索引越界或面板为空
 */
bool NavigationController::restorePanelByIndex(int index)
{
    if (index < 0 || index >= m_navPanelMappings.size()) {
        return false;
    }

    QWidget* target = m_navPanelMappings[index].widget;
    if (!target) return false;

    // 隐藏所有面板
    for (auto* w : allSwitchablePanels()) {
        if (w && w != target) {
            w->setVisible(false);
        }
    }

    // 直接显示目标面板（无动画）
    target->setVisible(true);
    m_currentPanel = target;

    return true;
}

/**
 * @brief 设置当前面板（初始化用，不触发动画）
 * @param panel 当前应显示的面板 widget
 */
void NavigationController::setCurrentPanel(QWidget* panel)
{
    m_currentPanel = panel;
}

/**
 * @brief 通过翻译后的名称查找对应的 panel widget
 * 使用 QCoreApplication::translate() 将裸字符串翻译后与导航树节点文本匹配
 * @param translatedName 导航树节点的翻译后文本
 * @return 匹配的面板 widget，未找到返回 nullptr
 */
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
 * @brief 面板切换（带并行交叉淡入淡出动画）
 *
 * 流程:
 * 1. 防重入检查（动画进行中忽略）
 * 2. 目标与当前面板相同则忽略
 * 3. 隐藏所有非目标、非旧面板（清除残留 opacity effect）
 * 4. 如果有旧面板: 并行执行 - 旧面板淡出(150ms) + 新面板淡入(150ms) 同时进行
 * 5. 如果无旧面板: 直接淡入新面板
 *
 * 并行模式 vs 旧串行模式:
 * - 旧: 淡出(150ms) → 等待完成 → 淡入(150ms) = 总计300ms，中间有空白闪烁
 * - 新: 淡出(150ms) + 淡入(150ms) 同时进行 = 总计150ms，无闪烁
 *
 * @param newPanel 目标面板 widget
 */
void NavigationController::switchToPanel(QWidget* newPanel)
{
    // 防止动画期间重复触发切换
    if (m_panelSwitching) return;

    // 如果目标是当前已显示的面板，无需切换
    if (m_currentPanel == newPanel) return;

    QWidget* oldPanel = m_currentPanel;

    // 更新当前面板追踪
    m_currentPanel = newPanel;

    // 隐藏所有非当前、非旧面板，并清除残留的 opacity effect
    for (auto* w : allSwitchablePanels()) {
        if (w && w != newPanel && w != oldPanel) {
            if (w->graphicsEffect()) {
                w->setGraphicsEffect(nullptr);
            }
            w->setVisible(false);
        }
    }

    if (!newPanel) return;

    // 如果有旧面板且旧面板可见，执行并行交叉淡入淡出:
    // 旧面板淡出的同时新面板淡入，总时长 150ms，消除串行闪烁
    if (oldPanel && oldPanel->isVisible()) {
        m_panelSwitching = true;

        // ---- 旧面板: 150ms InCubic opacity 1.0 → 0.0 淡出 ----
        QGraphicsOpacityEffect* fadeOutEffect = new QGraphicsOpacityEffect(oldPanel);
        oldPanel->setGraphicsEffect(fadeOutEffect);

        QPropertyAnimation* fadeOut = new QPropertyAnimation(fadeOutEffect, "opacity");
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        fadeOut->setDuration(150);
        fadeOut->setEasingCurve(QEasingCurve::InCubic);

        // 淡出完成后: 清除旧面板的 effect 并隐藏
        connect(fadeOut, &QPropertyAnimation::finished, this, [oldPanel]() {
            if (oldPanel->graphicsEffect()) {
                oldPanel->setGraphicsEffect(nullptr);
            }
            oldPanel->setVisible(false);
        });

        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);

        // ---- 新面板: 同时启动 150ms OutCubic opacity 0.0 → 1.0 淡入（并行） ----
        // 新面板需要叠在旧面板之上显示，先提升到最前再淡入
        newPanel->raise();
        fadeInPanel(newPanel);
    } else {
        // 无旧面板（首次切换或旧面板已隐藏），直接淡入新面板
        newPanel->setVisible(true);
        fadeInPanel(newPanel);
    }
}

/**
 * @brief 淡入动画辅助
 * 150ms OutCubic opacity 0.0 → 1.0
 * 动画完成后自动清除 QGraphicsOpacityEffect 以恢复正常绘制性能
 * @param panel 需要淡入的面板 widget
 */
void NavigationController::fadeInPanel(QWidget* panel)
{
    QGraphicsOpacityEffect* fadeInEffect = new QGraphicsOpacityEffect(panel);
    fadeInEffect->setOpacity(0.0);
    panel->setGraphicsEffect(fadeInEffect);

    // 首次切换时面板可能尚未可见
    if (!panel->isVisible()) {
        panel->setVisible(true);
    }

    QPropertyAnimation* fadeIn = new QPropertyAnimation(fadeInEffect, "opacity");
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setDuration(150);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    // 淡入完成后清除 effect，恢复正常绘制性能（避免 graphicsEffect 开销）
    connect(fadeIn, &QPropertyAnimation::finished, panel, [panel, fadeInEffect]() {
        if (panel->graphicsEffect() == fadeInEffect) {
            panel->setGraphicsEffect(nullptr);
        }
    });

    // 动画结束后重置切换标志并通知外部
    connect(fadeIn, &QPropertyAnimation::finished, this, [this, panel]() {
        m_panelSwitching = false;
        emit panelSwitched(panel);
    });

    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

/**
 * @brief 启动连接状态呼吸动画
 * 1500ms 循环, InOutSine, opacity 0.3 ↔ 1.0 脉冲闪烁
 * 用于"连接中..."状态，让用户知道正在连接而非卡死
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

    // 创建呼吸脉冲动画: 1500ms循环, InOutSine, opacity 0.3 ↔ 1.0
    if (m_breathingAnim) {
        m_breathingAnim->stop();
        delete m_breathingAnim;
    }
    m_breathingAnim = new QPropertyAnimation(m_connStatusEffect, "opacity");
    m_breathingAnim->setStartValue(0.3);
    m_breathingAnim->setEndValue(1.0);
    m_breathingAnim->setDuration(1500);
    m_breathingAnim->setEasingCurve(QEasingCurve::InOutSine);
    m_breathingAnim->setLoopCount(-1);  // 无限循环
    // 注意: loopCount=-1 时动画不会自行停止，不能用 DeleteWhenStopped
    // 生命周期由 startBreathingAnimation/stopBreathingAnimation 手动管理
    m_breathingAnim->start();
}

/**
 * @brief 停止连接状态呼吸动画
 * 停止动画 → 恢复标签完全不透明 → 清理 effect 和 anim 对象
 * @param statusLabel 状态标签控件（析构时可传 nullptr）
 */
void NavigationController::stopBreathingAnimation(QLabel* statusLabel)
{
    if (m_breathingAnim) {
        m_breathingAnim->stop();
        delete m_breathingAnim;
        m_breathingAnim = nullptr;
    }
    // 恢复状态标签完全不透明
    if (m_connStatusEffect) {
        m_connStatusEffect->setOpacity(1.0);
        if (statusLabel) {
            statusLabel->setGraphicsEffect(nullptr);
        }
        delete m_connStatusEffect;
        m_connStatusEffect = nullptr;
    }
}
