/**
 * @file TerminalWidgetSlots.cpp
 * @brief 自绘制终端控件 — 槽函数/右键菜单/统计计数
 *
 * 从 TerminalWidget.cpp 拆分而来，包含:
 *   - 模型数据回调槽函数(onDataAppended, onDataCleared)
 *   - 可见行范围更新(updateVisibleRange)
 *   - 右键菜单事件(contextMenuEvent)和全选(selectAll)
 *   - 统计计数器重置(resetTerminalWidgetStatistics)
 *
 * 输入事件处理(resizeEvent, wheelEvent, mousePress/Move/ReleaseEvent,
 * keyPressEvent)已拆分至 TerminalWidgetEvents.cpp。
 */

#include "terminal/widget/TerminalWidget.h"
#include <QContextMenuEvent>

// ---- 模型数据回调 ----
/** @brief 数据追加回调：更新缓存行计数，自动滚动到底部(锁定时仅更新按钮状态) @param firstNewLine 首行索引 @param count 新增行数 */
void TerminalWidget::onDataAppended(int firstNewLine, int count)
{
    Q_UNUSED(firstNewLine); Q_UNUSED(count);
    if (m_directionFilter->isFiltered()) { update(); return; }
    if (m_model) {
        m_maxScrollOffset = qMax(0, m_model->lineCount() - m_visibleLines);
        if (m_autoScroll) {
            m_scrollOffset = m_maxScrollOffset;
        }
    }
    updateScrollToBottomBtn();
    update();
}

/** @brief 数据清空回调：重置缓存和滚动位置，恢复自动滚动 */
void TerminalWidget::onDataCleared()
{
    m_cachedLines.clear(); m_cachedLineCount = 0;
    m_directionFilter->reset(); m_selectionManager->reset();
    m_scrollOffset = 0; m_maxScrollOffset = 0;
    if (!m_autoScroll) {
        m_autoScroll = true;
        emit autoScrollLockedChanged(false);
    }
    m_scrollToBottomBtn->hide();
    update();
}

/** @brief 计算并更新可见行范围(首行索引+可见行数+缓存构建) */
void TerminalWidget::updateVisibleRange()
{
    m_visibleLines = height() / m_lineHeight;
    if (m_model) {
        m_maxScrollOffset = m_directionFilter->isFiltered()
            ? qMax(0, m_directionFilter->filteredLineCount() - m_visibleLines)
            : qMax(0, m_model->lineCount() - m_visibleLines);
        m_scrollOffset = qMin(m_scrollOffset, m_maxScrollOffset);
    }
    updateScrollToBottomBtn();
}

// ---- 右键菜单 / 全选 ----
/** @brief 右键菜单事件：弹出复制/全选/清屏/搜索菜单 @param event 右键菜单事件 */
void TerminalWidget::contextMenuEvent(QContextMenuEvent* event)
{
    ++m_totalContextMenuActions;
    m_contextMenuManager->showContextMenu(event, !selectedText().isEmpty());
}

/** @brief 全选所有终端行文本 */
void TerminalWidget::selectAll()
{
    int totalLines = m_directionFilter->isFiltered()
        ? m_directionFilter->filteredLineCount()
        : (m_model ? m_model->lineCount() : 0);
    if (totalLines > 0 && m_selectionManager) {
        m_selectionManager->setSelection(0, totalLines - 1);
        update();
    }
}

// ---- 统计计数器 ----
/** @brief 重置终端统计计数器(渲染行数/按键/右键菜单/清屏/显示模式切换/匹配导航/自动滚动锁定) */
void TerminalWidget::resetTerminalWidgetStatistics()
{
    m_totalLinesRendered = 0;
    m_totalKeyPresses = 0;
    m_totalContextMenuActions = 0;
    m_totalClears = 0;
    m_totalDisplayModeChanges = 0;
    m_totalMatchNavigations = 0;
    m_totalAutoScrollLocks = 0;
    m_totalAutoScrollUnlocks = 0;
}

// ---- 自动滚动锁定管理 ----

/** @brief 判断用户是否在滚动区域底部(允许2行容差) @return true=在底部附近 */
bool TerminalWidget::isAtBottom() const
{
    return m_scrollOffset >= m_maxScrollOffset - 2;
}

/** @brief "滚动到底部"按钮点击槽: 解锁自动滚动并跳转到底部 */
void TerminalWidget::onScrollToBottomClicked()
{
    if (!m_autoScroll) {
        ++m_totalAutoScrollUnlocks;
        m_autoScroll = true;
        m_scrollOffset = m_maxScrollOffset;
        emit autoScrollLockedChanged(false);
    }
    updateScrollToBottomBtn();
    update();
}

/** @brief 更新"滚动到底部"浮动按钮的可见性和位置(右下角，距底部8px) */
void TerminalWidget::updateScrollToBottomBtn()
{
    if (m_autoScroll || m_maxScrollOffset <= 0) {
        m_scrollToBottomBtn->hide();
    } else {
        /* 定位到控件右下角，距底边距和右边距各8像素 */
        int btnX = width() - m_scrollToBottomBtn->width() - 8;
        int btnY = height() - m_scrollToBottomBtn->height() - 8;
        m_scrollToBottomBtn->move(btnX, btnY);
        m_scrollToBottomBtn->raise();
        m_scrollToBottomBtn->show();
    }
}
