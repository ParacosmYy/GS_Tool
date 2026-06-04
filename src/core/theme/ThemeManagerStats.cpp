/**
 * @file ThemeManagerStats.cpp
 * @brief ThemeManager 运行时统计接口实现
 *
 * 从 ThemeManagerApply.cpp 拆分而来，包含:
 *   - 所有统计计数器的查询接口 (totalXxx)
 *   - resetStats() 统计重置方法
 *
 * 主题查询/持久化/动画见 ThemeManagerApply.cpp。
 * 语义色板相关方法见 ThemeManagerColor.cpp。
 * 主题加载入口见 ThemeManager.cpp。
 */

#include "core/theme/ThemeManager.h"

// ==================== 统计接口 ====================

/** @brief 获取累计主题切换次数 */
quint64 ThemeManager::totalThemeSwitches() const
{
    return m_totalThemeSwitches;
}

/** @brief 获取累计自定义主题加载次数 */
quint64 ThemeManager::totalCustomThemesLoaded() const
{
    return m_totalCustomThemesLoaded;
}

/** @brief 获取累计主题重新加载次数(系统/保存主题) */
quint64 ThemeManager::totalThemeReloads() const
{
    return m_totalThemeReloads;
}

/** @brief 获取累计语义色查询次数 */
quint64 ThemeManager::totalColorQueries() const
{
    return m_totalColorQueries;
}

/** @brief 获取累计样式表应用次数 */
quint64 ThemeManager::totalStyleApplications() const
{
    return m_totalStyleApplications;
}

/** @brief 获取累计语义色缓存命中次数 @return 缓存命中次数 */
quint64 ThemeManager::totalCacheHits() const
{
    return m_totalCacheHits;
}

/** @brief 获取累计语义色缓存未命中次数 @return 缓存未命中次数 */
quint64 ThemeManager::totalCacheMisses() const
{
    return m_totalCacheMisses;
}

/** @brief 重置所有统计计数器为零 */
void ThemeManager::resetStats()
{
    m_totalThemeSwitches = 0;
    m_totalCustomThemesLoaded = 0;
    m_totalThemeReloads = 0;
    m_totalColorQueries = 0;
    m_totalStyleApplications = 0;
    m_totalCacheHits = 0;
    m_totalCacheMisses = 0;
}
