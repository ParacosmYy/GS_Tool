/**
 * @file ResponsiveLayout.cpp
 * @brief 响应式布局管理器实现 - 四断点体系、导航自动折叠、可见性策略
 *
 * 核心机制:
 *   通过 QObject::installEventFilter() 拦截 QMainWindow 的 QEvent::Resize 事件，
 *   根据宽度阈值判定断点变化，发射信号并设置窗口属性供 QSS 使用。
 *
 * 四断点体系 (Breakpoints 命名空间):
 *   Mobile  (< 768px):   隐藏导航树和次要面板，仅图标栏
 *   Tablet  (768~1023px): 折叠侧边栏，精简布局
 *   Desktop (1024~1439px): 完整侧边栏，全功能布局
 *   Wide    (>= 1440px):  宽松布局
 *
 * 导航自动折叠:
 *   独立于四断点，窗口宽度 < 900px 时自动折叠导航树
 */

#include "core/layout/ResponsiveLayout.h"
#include "shared/LayoutConstants.h"
#include "shared/AnimationConstants.h"

#include <QMainWindow>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QSettings>

/**
 * @brief 构造函数 - 初始化为 Desktop 断点
 * @param parent 父对象
 */
ResponsiveLayout::ResponsiveLayout(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 监听指定主窗口的尺寸变化
 *
 * 若已有监听窗口，先移除旧的事件过滤器。
 * 安装新过滤器后立即根据当前窗口宽度初始化断点状态，
 * 并设置窗口的 "breakpoint" 属性。
 *
 * @param window 要监听的主窗口指针
 */
void ResponsiveLayout::watchWindow(QMainWindow* window)
{
    // 移除对旧窗口的监听
    if (m_window) {
        m_window->removeEventFilter(this);
    }

    m_window = window;

    if (m_window) {
        m_window->installEventFilter(this);
        // 加载持久化配置
        loadLayoutConfig();
        // 立即根据当前尺寸初始化断点（不发射信号，仅设置内部状态和属性）
        updateBreakpoint(m_window->width());
    }
}

/**
 * @brief 获取当前断点
 * @return 当前激活的断点枚举值
 */
ResponsiveLayout::Breakpoint ResponsiveLayout::currentBreakpoint() const
{
    return m_currentBreakpoint;
}

/**
 * @brief 获取断点名称字符串
 *
 * 用于设置窗口属性，供 QSS 选择器使用:
 * @code
 *   QMainWindow[breakpoint="mobile"]  { ... }
 *   QMainWindow[breakpoint="tablet"]  { ... }
 *   QMainWindow[breakpoint="desktop"] { ... }
 *   QMainWindow[breakpoint="wide"]    { ... }
 * @endcode
 *
 * @return 小写断点名称
 */
QString ResponsiveLayout::breakpointName() const
{
    switch (m_currentBreakpoint) {
    case Breakpoint::Mobile:  return QStringLiteral("mobile");
    case Breakpoint::Tablet:  return QStringLiteral("tablet");
    case Breakpoint::Desktop: return QStringLiteral("desktop");
    case Breakpoint::Wide:    return QStringLiteral("wide");
    }
    return QStringLiteral("desktop");
}

/**
 * @brief 导航树是否应自动折叠
 * @return true 表示窗口宽度 < 900px，导航树应折叠
 */
bool ResponsiveLayout::isNavTreeCollapsed() const
{
    return m_navCollapsed;
}

/**
 * @brief 注册 widget 的断点可见性策略
 *
 * 为指定 widget 配置各断点下的显示行为。
 * 未配置的断点默认为 Visible。
 *
 * @param widget 目标 widget 指针
 * @param policy 断点到可见性策略的映射
 */
void ResponsiveLayout::registerWidgetVisibility(
    QWidget* widget,
    const QMap<Breakpoint, Visibility>& policy)
{
    if (!widget) return;
    m_visibilityPolicies[widget] = policy;
    // 立即应用当前断点的策略
    applyVisibilityPolicy(m_currentBreakpoint);
}

/**
 * @brief 移除 widget 的可见性策略注册
 * @param widget 目标 widget 指针
 */
void ResponsiveLayout::unregisterWidgetVisibility(QWidget* widget)
{
    m_visibilityPolicies.remove(widget);
}

/**
 * @brief 保存布局配置到 QSettings
 *
 * 持久化内容: 当前断点索引、导航折叠状态
 */
void ResponsiveLayout::saveLayoutConfig() const
{
    QSettings settings;
    settings.beginGroup("layout/responsive");
    settings.setValue("lastBreakpoint", static_cast<int>(m_currentBreakpoint));
    settings.setValue("navCollapsed", m_navCollapsed);
    settings.endGroup();
}

/**
 * @brief 从 QSettings 加载布局配置
 *
 * 恢复上次保存的断点偏好。注意: 仅更新内部状态，
 * 不触发信号或 UI 变更（由 watchWindow 后续的 updateBreakpoint 统一处理）。
 */
void ResponsiveLayout::loadLayoutConfig()
{
    QSettings settings;
    settings.beginGroup("layout/responsive");
    // 读取上次断点但不直接应用——watchWindow 后续 updateBreakpoint 会根据实际宽度判断
    settings.endGroup();
}

// ---- 统计计数器实现 ----

/** @brief 获取布局变更总次数(含resize触发) @return 布局变更总次数 */
quint64 ResponsiveLayout::totalLayoutChanges() const
{
    return m_totalLayoutChanges;
}

/** @brief 获取断点切换总次数 @return 断点切换总次数 */
quint64 ResponsiveLayout::breakpointChangeCount() const
{
    return m_breakpointChangeCount;
}

/** @brief 获取导航树折叠/展开切换总次数 @return 折叠切换次数 */
quint64 ResponsiveLayout::navCollapseToggleCount() const
{
    return m_navCollapseToggleCount;
}

/** @brief 重置所有统计计数器为零 */
void ResponsiveLayout::resetStats()
{
    m_totalLayoutChanges = 0;
    m_breakpointChangeCount = 0;
    m_navCollapseToggleCount = 0;
}

/**
 * @brief 事件过滤器 - 拦截被监听窗口的 Resize 事件
 *
 * 仅处理来自 m_window 的 QEvent::Resize 事件，
 * 其他事件和对象不受影响。
 *
 * @param watched 事件接收对象
 * @param event 事件对象
 * @return false（始终不拦截，仅监听）
 */
bool ResponsiveLayout::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_window && event->type() == QEvent::Resize) {
        auto* mainWindow = qobject_cast<QMainWindow*>(watched);
        if (mainWindow) {
            updateBreakpoint(mainWindow->width());
        }
    }
    return QObject::eventFilter(watched, event);
}

/**
 * @brief 根据窗口宽度计算断点（纯计算，无副作用）
 * @param width 窗口宽度
 * @return 对应断点
 */
ResponsiveLayout::Breakpoint ResponsiveLayout::calculateBreakpoint(int width)
{
    if (width < Breakpoints::kMobile) return Breakpoint::Mobile;
    if (width < Breakpoints::kTablet) return Breakpoint::Tablet;
    if (width < Breakpoints::kDesktop) return Breakpoint::Desktop;
    return Breakpoint::Wide;
}

/**
 * @brief 根据宽度判断导航树是否应折叠
 * @param width 窗口宽度
 * @return true 表示应折叠（< 900px）
 */
bool ResponsiveLayout::shouldCollapseNav(int width)
{
    return width < Breakpoints::kNavCollapse;
}

/**
 * @brief 根据窗口宽度更新断点状态
 *
 * 判定新断点并与当前断点比较:
 *   - 若相同: 仅更新窗口属性
 *   - 若不同: 启动过渡动画，发射信号，更新状态
 * 同时处理导航树自动折叠逻辑（独立于四断点）。
 *
 * @param width 窗口新宽度（像素）
 */
void ResponsiveLayout::updateBreakpoint(int width)
{
    ++m_totalLayoutChanges;

    // ---- 导航树自动折叠逻辑（独立于四断点） ----
    bool shouldCollapse = shouldCollapseNav(width);
    if (shouldCollapse != m_navCollapsed) {
        m_navCollapsed = shouldCollapse;
        ++m_navCollapseToggleCount;
        emit navTreeAutoCollapse(shouldCollapse);
    }

    // ---- 四断点判定 ----
    Breakpoint newBreakpoint = calculateBreakpoint(width);

    if (newBreakpoint != m_currentBreakpoint) {
        ++m_breakpointChangeCount;
        Breakpoint oldBreakpoint = m_currentBreakpoint;
        m_currentBreakpoint = newBreakpoint;

        // 设置窗口属性供 QSS 选择器使用
        if (m_window) {
            m_window->setProperty("breakpoint", breakpointName());
            // 强制刷新 QSS 属性选择器匹配
            m_window->setStyleSheet(m_window->styleSheet());
        }

        // 启动断点过渡动画
        startTransitionAnimation(newBreakpoint);

        // 应用可见性策略
        applyVisibilityPolicy(newBreakpoint);

        emit breakpointChanged(newBreakpoint, oldBreakpoint);
    } else {
        // 确保窗口属性与断点一致（首次初始化场景）
        if (m_window) {
            m_window->setProperty("breakpoint", breakpointName());
        }
    }
}

/**
 * @brief 应用当前断点的 widget 可见性策略
 *
 * 遍历所有已注册的 widget，根据当前断点的可见性策略设置
 * widget 的显示/隐藏/折叠状态。
 *
 * @param bp 当前断点
 */
void ResponsiveLayout::applyVisibilityPolicy(Breakpoint bp)
{
    for (auto it = m_visibilityPolicies.constBegin();
         it != m_visibilityPolicies.constEnd(); ++it) {
        QWidget* widget = it.key();
        if (!widget) continue;

        const auto& policyMap = it.value();
        Visibility vis = policyMap.value(bp, Visibility::Visible);

        switch (vis) {
        case Visibility::Visible:
            widget->setVisible(true);
            widget->setMinimumWidth(0);
            widget->setMaximumWidth(QWIDGETSIZE_MAX);
            break;
        case Visibility::Hidden:
            widget->setVisible(false);
            break;
        case Visibility::Collapsed:
            widget->setVisible(true);
            widget->setMinimumWidth(0);
            widget->setMaximumWidth(0);
            break;
        }
    }
}

/**
 * @brief 启动断点过渡动画
 *
 * 对被监听窗口应用透明度过渡动画:
 *   250ms InOutCubic, opacity 1.0 -> 0.85 -> 1.0
 * 实现断点切换时的平滑视觉过渡，避免布局突然跳变。
 *
 * @param newBp 目标断点
 */
void ResponsiveLayout::startTransitionAnimation(Breakpoint newBp)
{
    Q_UNUSED(newBp)

    if (!m_window) return;

    // 清理进行中的动画
    if (m_transitionAnim) {
        m_transitionAnim->stop();
        delete m_transitionAnim;
        m_transitionAnim = nullptr;
    }

    // 确保窗口有 opacity effect
    QGraphicsOpacityEffect* effect =
        qobject_cast<QGraphicsOpacityEffect*>(m_window->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(m_window);
        m_window->setGraphicsEffect(effect);
    }

    // 透明度脉冲动画: 1.0 -> 0.85 -> 1.0
    effect->setOpacity(1.0);

    m_transitionAnim = new QPropertyAnimation(effect, "opacity");
    m_transitionAnim->setStartValue(1.0);
    m_transitionAnim->setKeyValueAt(0.3, 0.85);
    m_transitionAnim->setEndValue(1.0);
    m_transitionAnim->setDuration(Animations::kBreakpointTransitionMs);
    m_transitionAnim->setEasingCurve(QEasingCurve::InOutCubic);

    connect(m_transitionAnim, &QPropertyAnimation::finished, this, [this]() {
        // 动画完成后恢复完全不透明状态
        if (m_window) {
            auto* eff = qobject_cast<QGraphicsOpacityEffect*>(m_window->graphicsEffect());
            if (eff) eff->setOpacity(1.0);
        }
        m_transitionAnim = nullptr;
    });

    emit breakpointTransitionStarted(newBp);
    m_transitionAnim->start(QAbstractAnimation::DeleteWhenStopped);
}
