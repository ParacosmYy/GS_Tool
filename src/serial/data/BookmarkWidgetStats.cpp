/**
 * @file BookmarkWidgetStats.cpp
 * @brief 书签管理面板 — 统计计数器查询与重置实现
 *
 * 从 BookmarkWidget.cpp 拆分而来，包含添加/删除/刷新/
 * 导航次数的统计 getter 和 resetBookmarkStats 方法。
 */

#include "serial/data/BookmarkWidget.h"

/** @brief 获取累计添加书签次数 @return 添加总次数 */
quint64 BookmarkWidget::totalBookmarksAdded() const { return m_totalBookmarksAdded; }

/** @brief 获取累计删除书签次数 @return 删除总次数 */
quint64 BookmarkWidget::totalBookmarksRemoved() const { return m_totalBookmarksRemoved; }

/** @brief 获取累计刷新次数 @return 刷新总次数 */
quint64 BookmarkWidget::totalRefreshes() const { return m_totalRefreshes; }

/** @brief 获取累计导航（双击跳转）次数 @return 导航总次数 */
quint64 BookmarkWidget::totalBookmarksNavigated() const { return m_totalBookmarksNavigated; }

/** @brief 重置统计计数器(添加/删除/刷新/导航次数归零) */
void BookmarkWidget::resetBookmarkStats()
{
    m_totalBookmarksAdded = 0;
    m_totalBookmarksRemoved = 0;
    m_totalRefreshes = 0;
    m_totalBookmarksNavigated = 0;
}
