/**
 * @file BasePanel.cpp
 * @brief BasePanel容器组件实现 — 统一面板标题栏、折叠和动画
 *
 * 实现细节:
 *   - 标题栏高度36px, 图标16px, 折叠按钮24px
 *   - 内容区域边距 12/8/12/12
 *   - 动画: show(250ms OutCubic), hide(200ms InCubic)
 *   - 阴影: paintEvent绘制底部/右侧微阴影(ThemeManager::Shadow)
 *   - m_content不拥有, 由PanelManager的父树管理生命周期
 */

#include "core/widgets/BasePanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include "core/theme/ThemeManager.h"
#include "core/theme/Constants.h"
#include "core/widgets/EmptyStateWidget.h"
#include "core/widgets/LoadingSpinner.h"
#include "core/widgets/SkeletonWidget.h"

// ============================================================================
// 构造/析构
// ============================================================================

BasePanel::BasePanel(QWidget* content, const QString& title, QWidget* parent)
    : QWidget(parent)
    , m_content(content)
{
    setObjectName("basePanel");
    setupInternalLayout();
    setTitle(title);

    // 透明度特效，供 animateShow/Hide 使用
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(1.0);
    setGraphicsEffect(m_opacityEffect);
}

// ============================================================================
// 公开接口
// ============================================================================

QWidget* BasePanel::contentWidget() const
{
    return m_content;
}

void BasePanel::setTitle(const QString& title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

QString BasePanel::title() const
{
    return m_titleLabel ? m_titleLabel->text() : QString();
}

void BasePanel::setIconName(const QString& name)
{
    m_iconName = name;
    if (m_iconLabel) {
        // 占位: 显示首字符，后续接入IconManager替换
        m_iconLabel->setText(name.isEmpty() ? QString() : QString(name.at(0)));
    }
}

void BasePanel::setCollapsible(bool enabled)
{
    m_collapsible = enabled;
    if (m_collapseBtn) {
        m_collapseBtn->setVisible(enabled);
    }
    // 禁止折叠时，确保内容区域可见
    if (!enabled && m_collapsed) {
        setCollapsed(false);
    }
}

bool BasePanel::isCollapsible() const
{
    return m_collapsible;
}

void BasePanel::setCollapsed(bool collapsed)
{
    if (collapsed == m_collapsed) return;
    m_collapsed = collapsed;

    if (m_contentArea) {
        m_contentArea->setVisible(!collapsed);
    }
    updateCollapseIcon();
    emit collapsedChanged(collapsed);
}

bool BasePanel::isCollapsed() const
{
    return m_collapsed;
}

qreal BasePanel::panelOpacity() const
{
    return m_panelOpacity;
}

void BasePanel::setPanelOpacity(qreal opacity)
{
    m_panelOpacity = opacity;
    if (m_opacityEffect) {
        m_opacityEffect->setOpacity(opacity);
    }
}

// ============================================================================
// 空状态 / 加载状态
// ============================================================================

void BasePanel::showEmptyState(const QString& title, const QString& description)
{
    if (m_emptyState) {
        m_emptyState->setTitle(title);
        m_emptyState->setDescription(description);
        m_emptyState->setVisible(true);
    }
    if (m_content) m_content->setVisible(false);
    if (m_loadingSpinner) m_loadingSpinner->setVisible(false);
    if (m_skeleton) m_skeleton->setVisible(false);
}

void BasePanel::hideEmptyState()
{
    if (m_emptyState) m_emptyState->setVisible(false);
    if (m_content) m_content->setVisible(true);
}

void BasePanel::showLoading()
{
    if (m_loadingSpinner) m_loadingSpinner->setVisible(true);
    if (m_content) m_content->setVisible(false);
    if (m_emptyState) m_emptyState->setVisible(false);
    if (m_skeleton) m_skeleton->setVisible(false);
}

void BasePanel::hideLoading()
{
    if (m_loadingSpinner) m_loadingSpinner->setVisible(false);
    if (m_content) m_content->setVisible(true);
}

void BasePanel::showSkeleton()
{
    if (m_skeleton) m_skeleton->setVisible(true);
    if (m_content) m_content->setVisible(false);
    if (m_emptyState) m_emptyState->setVisible(false);
    if (m_loadingSpinner) m_loadingSpinner->setVisible(false);
}

void BasePanel::hideSkeleton()
{
    if (m_skeleton) m_skeleton->setVisible(false);
    if (m_content) m_content->setVisible(true);
}

// ============================================================================
// 动画槽
// ============================================================================

void BasePanel::animateShow()
{
    show();
    setPanelOpacity(0.0);

    auto* group = new QParallelAnimationGroup(this);

    // 淡入: 0 → 1, 250ms OutCubic
    auto* fadeIn = new QPropertyAnimation(this, "panelOpacity");
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setDuration(Animations::kPanelSlideInMs);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    group->addAnimation(fadeIn);

    // 滑入: Y -20 → 0, 250ms OutCubic
    auto* slideIn = new QPropertyAnimation(this, "pos");
    QPoint basePos = pos();
    slideIn->setStartValue(basePos + QPoint(0, -20));
    slideIn->setEndValue(basePos);
    slideIn->setDuration(Animations::kPanelSlideInMs);
    slideIn->setEasingCurve(QEasingCurve::OutCubic);
    group->addAnimation(slideIn);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void BasePanel::animateHide()
{
    auto* group = new QParallelAnimationGroup(this);

    // 淡出: 1 → 0, 200ms InCubic
    auto* fadeOut = new QPropertyAnimation(this, "panelOpacity");
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setDuration(Animations::kPanelSlideOutMs);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);
    group->addAnimation(fadeOut);

    // 滑出: Y 0 → -20, 200ms InCubic
    auto* slideOut = new QPropertyAnimation(this, "pos");
    QPoint basePos = pos();
    slideOut->setStartValue(basePos);
    slideOut->setEndValue(basePos + QPoint(0, -20));
    slideOut->setDuration(Animations::kPanelSlideOutMs);
    slideOut->setEasingCurve(QEasingCurve::InCubic);
    group->addAnimation(slideOut);

    connect(group, &QAbstractAnimation::finished, this, &QWidget::hide);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================================
// 自绘: 微阴影
// ============================================================================

void BasePanel::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 从ThemeManager获取阴影色
    QColor shadowColor = ThemeManager::instance().color(
        ThemeManager::SemanticColor::Shadow);

    const int w = width();
    const int h = height();
    const int shadowSize = 6;

    // 底部阴影渐变
    QLinearGradient bottomGrad(0, h - shadowSize, 0, h);
    QColor cBot = shadowColor;
    cBot.setAlpha(40);
    bottomGrad.setColorAt(0.0, QColor(cBot.red(), cBot.green(), cBot.blue(), 0));
    bottomGrad.setColorAt(1.0, cBot);
    p.fillRect(0, h - shadowSize, w, shadowSize, bottomGrad);

    // 右侧阴影渐变
    QLinearGradient rightGrad(w - shadowSize, 0, w, 0);
    QColor cRight = shadowColor;
    cRight.setAlpha(25);
    rightGrad.setColorAt(0.0, QColor(cRight.red(), cRight.green(), cRight.blue(), 0));
    rightGrad.setColorAt(1.0, cRight);
    p.fillRect(w - shadowSize, 0, shadowSize, h, rightGrad);
}

// ============================================================================
// 私有方法
// ============================================================================

void BasePanel::setupInternalLayout()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 标题栏 (36px固定高度) ---
    m_headerBar = new QWidget(this);
    m_headerBar->setObjectName("panelHeaderBar");
    m_headerBar->setFixedHeight(36);

    auto* headerLayout = new QHBoxLayout(m_headerBar);
    headerLayout->setContentsMargins(12, 0, 8, 0);
    headerLayout->setSpacing(8);

    // 图标 (16x16占位)
    m_iconLabel = new QLabel(m_headerBar);
    m_iconLabel->setObjectName("panelIcon");
    m_iconLabel->setFixedSize(16, 16);
    headerLayout->addWidget(m_iconLabel);

    // 标题文字
    m_titleLabel = new QLabel(m_headerBar);
    m_titleLabel->setObjectName("panelTitle");
    headerLayout->addWidget(m_titleLabel);

    headerLayout->addStretch();

    // 折叠按钮 (24x24)
    m_collapseBtn = new QPushButton(m_headerBar);
    m_collapseBtn->setObjectName("collapseButton");
    m_collapseBtn->setFixedSize(24, 24);
    m_collapseBtn->setText(QStringLiteral("▾")); // 占位，后续用IconManager
    connect(m_collapseBtn, &QPushButton::clicked,
            this, &BasePanel::toggleCollapsed);
    headerLayout->addWidget(m_collapseBtn);

    mainLayout->addWidget(m_headerBar);

    // --- 内容区域 ---
    m_contentArea = new QWidget(this);
    m_contentArea->setObjectName("panelContentArea");

    auto* contentLayout = new QVBoxLayout(m_contentArea);
    contentLayout->setContentsMargins(12, 8, 12, 12);
    contentLayout->setSpacing(0);

    if (m_content) {
        m_content->setParent(m_contentArea);
        contentLayout->addWidget(m_content);
    }

    // 空状态占位组件(默认隐藏，showEmptyState时显示)
    m_emptyState = new EmptyStateWidget(tr("暂无数据"), tr("等待数据..."), m_contentArea);
    m_emptyState->setObjectName("panelEmptyState");
    m_emptyState->setVisible(false);
    contentLayout->addWidget(m_emptyState);

    // 加载旋转指示器(默认隐藏，showLoading时显示)
    m_loadingSpinner = new LoadingSpinner(32, m_contentArea);
    m_loadingSpinner->setObjectName("panelLoadingSpinner");
    m_loadingSpinner->setVisible(false);
    contentLayout->addWidget(m_loadingSpinner, 0, Qt::AlignCenter);

    // 骨架屏容器(默认隐藏，showSkeleton时显示)
    auto* skeletonContainer = new QWidget(m_contentArea);
    skeletonContainer->setObjectName("panelSkeleton");
    skeletonContainer->setVisible(false);
    auto* skeletonLayout = new QVBoxLayout(skeletonContainer);
    skeletonLayout->setContentsMargins(0, 4, 0, 0);
    skeletonLayout->setSpacing(8);

    // 标题占位行(较窄)
    auto* titleSkeleton = new SkeletonWidget(200, 16, 4, skeletonContainer);
    titleSkeleton->setObjectName("skeletonTitle");
    skeletonLayout->addWidget(titleSkeleton);

    // 内容占位行×2(较宽)
    auto* row1 = new SkeletonWidget(0, 14, 4, skeletonContainer);
    row1->setObjectName("skeletonRow1");
    skeletonLayout->addWidget(row1);

    auto* row2 = new SkeletonWidget(0, 14, 4, skeletonContainer);
    row2->setObjectName("skeletonRow2");
    skeletonLayout->addWidget(row2);

    skeletonLayout->addStretch();
    contentLayout->addWidget(skeletonContainer);
    m_skeleton = skeletonContainer;

    mainLayout->addWidget(m_contentArea, 1);
}

void BasePanel::updateCollapseIcon()
{
    if (!m_collapseBtn) return;
    // 折叠: ▸ 展开: ▾
    m_collapseBtn->setText(m_collapsed ? QStringLiteral("▸")
                                       : QStringLiteral("▾"));
}

void BasePanel::toggleCollapsed()
{
    setCollapsed(!m_collapsed);
}
