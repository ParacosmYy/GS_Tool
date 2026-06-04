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

// ---- 查询/统计/配置方法已拆分至 ResponsiveLayoutQuery.cpp ----

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

// ---- applyVisibilityPolicy/startTransitionAnimation见 ResponsiveLayoutTransition.cpp ----
