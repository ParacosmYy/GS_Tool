#include "NavigationController.h"
#include "Constants.h"
#include <QTreeView>
#include <QStandardItemModel>
#include <QPainter>
#include <QPixmap>
#include <QLabel>

// 创建导航树连接类型指示圆点图标（8x8透明底+抗锯齿彩色圆）
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

NavigationController::NavigationController(QObject* parent)
    : QObject(parent)
{
}

NavigationController::~NavigationController()
{
    stopBreathingAnimation(nullptr);
}

void NavigationController::buildNavTree(QTreeView* navTree, const QVector<NavPanelMapping>& mappings)
{
    m_navPanelMappings = mappings;

    auto* treeModel = new QStandardItemModel(this);
    auto* rootItem = treeModel->invisibleRootItem();

    // 串口分组 — 蓝色圆点
    auto* serialItem = new QStandardItem(createDotIcon(QColor(NavColors::kSerialDot)), tr("串口"));
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

    // 网络分组
    auto* networkItem = new QStandardItem(tr("网络"));
    networkItem->setEditable(false);
    // TCP客户端/服务端 — 绿色圆点
    auto* tcpClientItem = new QStandardItem(createDotIcon(QColor(NavColors::kTcpDot)), tr("TCP客户端"));
    tcpClientItem->setEditable(false);
    auto* tcpServerItem = new QStandardItem(createDotIcon(QColor(NavColors::kTcpDot)), tr("TCP服务端"));
    tcpServerItem->setEditable(false);
    // UDP — 黄色圆点
    auto* udpItem = new QStandardItem(createDotIcon(QColor(NavColors::kUdpDot)), tr("UDP"));
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
    navTree->expandAll();
}

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

void NavigationController::setCurrentPanel(QWidget* panel)
{
    m_currentPanel = panel;
}

QWidget* NavigationController::lookupPanel(const QString& translatedName) const
{
    for (const auto& mapping : m_navPanelMappings) {
        if (translatedName == tr(mapping.name)) {
            return mapping.widget;
        }
    }
    return nullptr;
}

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

    // 如果有旧面板且旧面板可见，执行: 淡出旧面板 → 显示新面板 → 淡入新面板
    if (oldPanel && oldPanel->isVisible()) {
        m_panelSwitching = true;

        // 旧面板: 200ms InCubic opacity 1.0 → 0.0 淡出
        QGraphicsOpacityEffect* fadeOutEffect = new QGraphicsOpacityEffect(oldPanel);
        oldPanel->setGraphicsEffect(fadeOutEffect);

        QPropertyAnimation* fadeOut = new QPropertyAnimation(fadeOutEffect, "opacity");
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        fadeOut->setDuration(200);
        fadeOut->setEasingCurve(QEasingCurve::InCubic);

        // 淡出完成后: 隐藏旧面板 → 显示新面板 → 淡入新面板
        connect(fadeOut, &QPropertyAnimation::finished, this, [this, oldPanel, newPanel]() {
            // 清除旧面板的 effect 并隐藏
            if (oldPanel->graphicsEffect()) {
                oldPanel->setGraphicsEffect(nullptr);
            }
            oldPanel->setVisible(false);

            // 淡入新面板
            fadeInPanel(newPanel);
        });

        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        // 无旧面板（首次切换或旧面板已隐藏），直接淡入新面板
        newPanel->setVisible(true);
        fadeInPanel(newPanel);
    }
}

void NavigationController::fadeOutPanel(QWidget* panel, QPropertyAnimation* anim)
{
    Q_UNUSED(panel);
    Q_UNUSED(anim);
    // fadeOut 逻辑已内联在 switchToPanel 中，此方法预留用于未来扩展
}

void NavigationController::fadeInPanel(QWidget* panel)
{
    // 新面板: 250ms OutCubic opacity 0.0 → 1.0 淡入
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
    fadeIn->setDuration(250);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    // 淡入完成后清除 effect，恢复正常绘制性能
    connect(fadeIn, &QPropertyAnimation::finished, panel, [panel, fadeInEffect]() {
        if (panel->graphicsEffect() == fadeInEffect) {
            panel->setGraphicsEffect(nullptr);
        }
    });

    connect(fadeIn, &QPropertyAnimation::finished, this, [this]() {
        m_panelSwitching = false;
    });

    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

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
    // 注意: loopCount=-1 时动画不会自行停止，所以不能用 DeleteWhenStopped。
    // 生命周期由 startBreathingAnimation/stopBreathingAnimation 手动管理。
    m_breathingAnim->start();
}

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
