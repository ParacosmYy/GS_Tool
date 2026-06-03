/**
 * @file ResponsiveLayout.cpp
 * @brief 响应式布局管理器实现 - 窗口断点监测与通知
 *
 * 核心机制:
 *   通过 QObject::installEventFilter() 拦截 QMainWindow 的 QEvent::Resize 事件，
 *   根据宽度阈值判定断点变化，发射信号并设置窗口属性供 QSS 使用。
 */

#include "core/layout/ResponsiveLayout.h"

#include <QMainWindow>
#include <QEvent>

/**
 * @brief 断点阈值常量
 *
 * Compact:  width < 900
 * Medium:   900 <= width < 1200
 * Desktop:  width >= 1200
 */
static constexpr int BREAKPOINT_COMPACT  = 900;
static constexpr int BREAKPOINT_DESKTOP  = 1200;

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
 *   QMainWindow[breakpoint="compact"] { ... }
 *   QMainWindow[breakpoint="medium"]  { ... }
 *   QMainWindow[breakpoint="desktop"] { ... }
 * @endcode
 *
 * @return 小写断点名称
 */
QString ResponsiveLayout::breakpointName() const
{
    switch (m_currentBreakpoint) {
    case Breakpoint::Compact: return QStringLiteral("compact");
    case Breakpoint::Medium:  return QStringLiteral("medium");
    case Breakpoint::Desktop: return QStringLiteral("desktop");
    }
    return QStringLiteral("desktop");
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

/** @brief 重置所有统计计数器为零 */
void ResponsiveLayout::resetStats()
{
    m_totalLayoutChanges = 0;
    m_breakpointChangeCount = 0;
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
 * @brief 根据窗口宽度更新断点状态
 *
 * 判定新断点并与当前断点比较:
 *   - 若相同: 仅更新窗口属性（确保属性一致）
 *   - 若不同: 发射 breakpointChanged 信号，更新内部状态和窗口属性
 *
 * 窗口属性 "breakpoint" 设置为 breakpointName() 返回的小写字符串，
 * 供 QSS 选择器动态匹配。
 *
 * @param width 窗口新宽度（像素）
 */
void ResponsiveLayout::updateBreakpoint(int width)
{
    ++m_totalLayoutChanges;  ///< 统计: 每次resize触发布局变更计数递增

    // 判定新断点
    Breakpoint newBreakpoint;
    if (width < BREAKPOINT_COMPACT) {
        newBreakpoint = Breakpoint::Compact;
    } else if (width < BREAKPOINT_DESKTOP) {
        newBreakpoint = Breakpoint::Medium;
    } else {
        newBreakpoint = Breakpoint::Desktop;
    }

    // 断点变化时发射信号
    if (newBreakpoint != m_currentBreakpoint) {
        ++m_breakpointChangeCount;  ///< 统计: 断点实际切换次数递增
        Breakpoint oldBreakpoint = m_currentBreakpoint;
        m_currentBreakpoint = newBreakpoint;

        // 设置窗口属性供 QSS 选择器使用
        if (m_window) {
            m_window->setProperty("breakpoint", breakpointName());
            // 强制刷新 QSS 属性选择器匹配
            m_window->setStyleSheet(m_window->styleSheet());
        }

        emit breakpointChanged(newBreakpoint, oldBreakpoint);
    } else {
        // 确保窗口属性与断点一致（首次初始化场景）
        if (m_window) {
            m_window->setProperty("breakpoint", breakpointName());
        }
    }
}
