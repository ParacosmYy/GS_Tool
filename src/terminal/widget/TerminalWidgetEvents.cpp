/**
 * @file TerminalWidgetEvents.cpp
 * @brief 自绘制终端控件 — 输入事件处理实现
 *
 * 从 TerminalWidgetSlots.cpp 拆分而来，专注于用户输入事件的分发与响应：
 *   - resizeEvent: 窗口大小变化时更新可见行范围
 *   - wheelEvent: 滚轮上下滚动终端内容，支持累积滚动与自动滚屏取消
 *   - mousePressEvent: 鼠标按下时记录文本选区起点
 *   - mouseMoveEvent: 鼠标拖动时实时更新文本选区范围
 *   - mouseReleaseEvent: 鼠标释放时完成文本选区
 *   - keyPressEvent: 键盘快捷键处理(Ctrl+C/V/A/F、F3导航)
 *
 * 槽函数/右键菜单/统计计数保留在 TerminalWidgetSlots.cpp 中。
 */

#include "terminal/widget/TerminalWidget.h"
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

// ---- 窗口大小事件 ----
/** @brief 窗口大小变化事件：更新可见行范围 @param event 大小变化事件 */
void TerminalWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); updateVisibleRange(); }

// ---- 鼠标事件 ----
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

// ---- 键盘事件 ----
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
