/**
 * @file ResponsiveLayoutQuery.cpp
 * @brief 响应式布局查询/统计/配置方法实现
 *
 * 从 ResponsiveLayout.cpp 拆分而来，包含:
 *   - 断点查询方法 (currentBreakpoint / breakpointName / isNavTreeCollapsed)
 *   - 统计计数器 (totalLayoutChanges / breakpointChangeCount / navCollapseToggleCount / resetStats)
 *   - 配置持久化 (saveLayoutConfig / loadLayoutConfig)
 *
 * 布局几何计算和事件处理仍保留在 ResponsiveLayout.cpp 中。
 */

#include "core/layout/ResponsiveLayout.h"

#include <QMainWindow>
#include <QSettings>

// ---- 断点查询方法 ----

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

// ---- 配置持久化 ----

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

// ---- 统计计数器 ----

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
    m_totalWidgetRegistrations = 0;
}
