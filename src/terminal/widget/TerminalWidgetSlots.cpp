/**
 * @file TerminalWidgetSlots.cpp
 * @brief 自绘制终端控件 — 事件处理/槽函数/右键菜单/统计计数
 *
 * 从 TerminalWidget.cpp 拆分而来，包含:
 *   - 窗口大小变化事件(resizeEvent)
 *   - 鼠标滚轮/按下/移动/释放事件(wheelEvent, mousePress/Move/ReleaseEvent)
 *   - 键盘事件处理(keyPressEvent: Ctrl+C/V/A/F, F3导航)
 *   - 模型数据回调槽函数(onDataAppended, onDataCleared)
 *   - 可见行范围更新(updateVisibleRange)
 *   - 右键菜单事件(contextMenuEvent)和全选(selectAll)
 *   - 统计计数器重置(resetTerminalWidgetStatistics)
 */

#include "terminal/widget/TerminalWidget.h"
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>

// ---- 事件处理 ----
/** @brief 窗口大小变化事件：更新可见行范围 @param event 大小变化事件 */
void TerminalWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); updateVisibleRange(); }

/** @brief 鼠标滚轮事件：Ctrl+滚轮缩放字号，普通滚轮上下滚动 @param event 滚轮事件 */
void TerminalWidget::wheelEvent(QWheelEvent* event)
{
    int delta = event->angleDelta().y();
    m_scrollAccumulator += delta;
    int lines = m_scrollAccumulator / 40;
    if (lines != 0) {
        m_scrollAccumulator -= lines * 40;
        m_scrollOffset -= lines;
    }
    m_scrollOffset = qMax(0, qMin(m_scrollOffset, m_maxScrollOffset));
    // 任何手动滚动只要不在底部就禁用自动滚动（修复向上滚动立即回弹问题）
    if (m_scrollOffset < m_maxScrollOffset) m_autoScroll = false;
    update();
    event->accept();
}

/** @brief 鼠标按下事件：记录选区起点 @param event 鼠标事件 */
void TerminalWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_selectionManager->onMousePress(event->position().y(), m_scrollOffset, m_lineHeight);
        update();
    }
    QWidget::mousePressEvent(event);
}

/** @brief 鼠标移动事件：更新文本选区并重绘 @param event 鼠标事件 */
void TerminalWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_selectionManager->onMouseMove(event->position().y(), m_scrollOffset, m_lineHeight);
        update();
    }
    QWidget::mouseMoveEvent(event);
}

/** @brief 鼠标释放事件：完成文本选区，自动复制选中内容到剪贴板 @param event 鼠标事件 */
void TerminalWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) m_selectionManager->onMouseRelease();
    QWidget::mouseReleaseEvent(event);
}

/** @brief 键盘事件：Ctrl+C复制、Ctrl+F搜索、F3/Shift+F3导航匹配、Ctrl+A全选 @param event 键盘事件 */
void TerminalWidget::keyPressEvent(QKeyEvent* event)
{
    ++m_totalKeyPresses;

    // Ctrl+C 复制选中内容 — 精确匹配 Ctrl 修饰键，避免 Ctrl+Shift+C 被误拦截
    if (event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier) {
        QString text = selectedText();
        if (!text.isEmpty()) QApplication::clipboard()->setText(text);
        return;
    }
    // Ctrl+A 全选 — 精确匹配，避免 Ctrl+Shift+A 误触发
    if (event->key() == Qt::Key_A && event->modifiers() == Qt::ControlModifier) {
        selectAll();
        return;
    }
    // Ctrl+V 粘贴 — 精确匹配，转发粘贴请求
    if (event->key() == Qt::Key_V && event->modifiers() == Qt::ControlModifier) {
        QString text = QApplication::clipboard()->text();
        if (!text.isEmpty()) emit pasteRequested(text);
        return;
    }
    // Ctrl+F 搜索 — 精确匹配，避免 Ctrl+Shift+F 误触发
    if (event->key() == Qt::Key_F && event->modifiers() == Qt::ControlModifier) {
        emit searchRequested();
        return;
    }
    // F3 / Shift+F3 搜索导航
    if (event->key() == Qt::Key_F3) {
        (event->modifiers() & Qt::ShiftModifier) ? gotoPrevMatch() : gotoNextMatch();
        return;
    }
    QWidget::keyPressEvent(event);
}

// ---- 模型数据回调 ----
/** @brief 数据追加回调：更新缓存行计数，自动滚动到底部 @param firstNewLine 首行索引 @param count 新增行数 */
void TerminalWidget::onDataAppended(int firstNewLine, int count)
{
    Q_UNUSED(firstNewLine); Q_UNUSED(count);
    if (m_directionFilter->isFiltered()) { update(); return; }
    if (m_model) {
        m_maxScrollOffset = qMax(0, m_model->lineCount() - m_visibleLines);
        if (m_autoScroll) m_scrollOffset = m_maxScrollOffset;
    }
    update();
}

/** @brief 数据清空回调：重置缓存和滚动位置 */
void TerminalWidget::onDataCleared()
{
    m_cachedLines.clear(); m_cachedLineCount = 0;
    m_directionFilter->reset(); m_selectionManager->reset();
    m_scrollOffset = 0; m_maxScrollOffset = 0;
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
/** @brief 重置终端统计计数器(渲染行数/按键/右键菜单操作) */
void TerminalWidget::resetTerminalWidgetStatistics()
{
    m_totalLinesRendered = 0;
    m_totalKeyPresses = 0;
    m_totalContextMenuActions = 0;
}
