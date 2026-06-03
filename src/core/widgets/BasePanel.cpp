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
#include "core/widgets/EmptyStateWidget.h"
#include "core/widgets/LoadingSpinner.h"
#include "core/widgets/SkeletonWidget.h"
#include "shared/AnimationConstants.h"

// ============================================================================
// 构造/析构
// ============================================================================

/** @brief 构造BasePanel容器面板 @param content 内容区域控件指针(不转移所有权) @param title 标题栏文字 @param parent 父控件指针 */
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

/** @brief 获取面板内容区域控件指针 @return 内容控件指针，可能为nullptr */
QWidget* BasePanel::contentWidget() const
{
    return m_content;
}

/** @brief 设置标题栏文字 @param title 新的标题文字 */
void BasePanel::setTitle(const QString& title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

/** @brief 获取当前标题栏文字 @return 标题文字，标签未创建时返回空字符串 */
QString BasePanel::title() const
{
    return m_titleLabel ? m_titleLabel->text() : QString();
}

/** @brief 设置标题栏图标名称，显示首字符作为占位 @param name 图标名称 */
void BasePanel::setIconName(const QString& name)
{
    m_iconName = name;
    if (m_iconLabel) {
        // 占位: 显示首字符，后续接入IconManager替换
        m_iconLabel->setText(name.isEmpty() ? QString() : QString(name.at(0)));
    }
}

/** @brief 设置面板是否可折叠 @param enabled true允许折叠，false禁止折叠 */
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

/** @brief 查询面板是否可折叠 @return true表示允许折叠 */
bool BasePanel::isCollapsible() const
{
    return m_collapsible;
}

/** @brief 设置面板折叠状态 @param collapsed true折叠内容区域，false展开内容区域 */
void BasePanel::setCollapsed(bool collapsed)
{
    if (collapsed == m_collapsed) return;
    m_collapsed = collapsed;

    if (collapsed) ++m_totalCollapses; else ++m_totalExpansions;

    if (m_contentArea) {
        m_contentArea->setVisible(!collapsed);
    }
    updateCollapseIcon();
    emit collapsedChanged(collapsed);
}

/** @brief 查询面板当前是否处于折叠状态 @return true表示已折叠 */
bool BasePanel::isCollapsed() const
{
    return m_collapsed;
}

/** @brief 获取面板透明度 @return 透明度值(0.0~1.0) */
qreal BasePanel::panelOpacity() const
{
    return m_panelOpacity;
}

/** @brief 设置面板透明度 @param opacity 透明度值(0.0完全透明~1.0完全不透明) */
void BasePanel::setPanelOpacity(qreal opacity)
{
    m_panelOpacity = opacity;
    if (m_opacityEffect) {
        m_opacityEffect->setOpacity(opacity);
    }
}

// 状态切换、动画和自绘见 BasePanelStates.cpp

// ============================================================================
// 私有方法
// ============================================================================

/** @brief 初始化面板内部布局：标题栏(图标+标题+折叠按钮) + 内容区域(含空状态/加载/骨架屏) */
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
    m_collapseBtn->setText(tr("▾")); // 占位，后续用IconManager
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

/** @brief 根据当前折叠状态更新折叠按钮图标(▸折叠/▾展开) */
void BasePanel::updateCollapseIcon()
{
    if (!m_collapseBtn) return;
    // 折叠: ▸ 展开: ▾
    m_collapseBtn->setText(m_collapsed ? tr("▸")
                                       : tr("▾"));
}

/** @brief 切换面板折叠/展开状态 */
void BasePanel::toggleCollapsed()
{
    ++m_totalToggles;
    setCollapsed(!m_collapsed);
}

// 统计见 BasePanelStates.cpp
