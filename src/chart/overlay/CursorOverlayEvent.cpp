/**
 * @file CursorOverlayEvent.cpp
 * @brief 波形游标叠加层 - 事件过滤与主题切换实现
 *
 * 从 CursorOverlay.cpp 拆分而来，包含事件过滤器(双击/右键放置、
 * 左键拖拽)和主题切换颜色刷新方法。
 */

#include "chart/overlay/CursorOverlay.h"
#include "core/theme/ThemeManager.h"

#include <QMouseEvent>

/**
 * @brief 事件过滤器主入口
 * @param watched 被观察的对象
 * @param event 事件对象
 * @return true=事件已消费(命中游标)，false=放行给ZoomController
 *
 * 处理规则:
 * - DoubleClick + LeftButton → 放置游标A（始终消费）
 * - Press + RightButton → 放置游标B（始终消费）
 * - Press + LeftButton → 命中游标则开始拖拽（消费），否则放行
 * - Move → 拖拽中则移动游标（消费），否则放行
 * - Release → 结束拖拽（消费），否则放行
 */
bool CursorOverlay::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)

    /* 游标未激活时不拦截任何事件 */
    if (!isVisible()) return false;

    switch (event->type()) {
    case QEvent::MouseButtonDblClick: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            setCursorA(pixelToDataX(static_cast<int>(me->position().x())));
            ++m_totalCursorToggles;
            return true;
        }
        break;
    }
    case QEvent::MouseButtonPress: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::RightButton) {
            setCursorB(pixelToDataX(static_cast<int>(me->position().x())));
            ++m_totalCursorToggles;
            return true;
        }
        if (me->button() == Qt::LeftButton) {
            int hit = hitTestCursor(static_cast<int>(me->position().x()));
            ++m_totalHitTests;
            if (hit > 0) {
                m_draggingCursor = hit;
                return true;
            }
        }
        break;
    }
    case QEvent::MouseMove: {
        if (m_draggingCursor > 0) {
            auto* me = static_cast<QMouseEvent*>(event);
            int px = static_cast<int>(me->position().x());
            if (m_draggingCursor == 1) {
                m_cursorAX = pixelToDataX(px);
            } else {
                m_cursorBX = pixelToDataX(px);
            }
            ++m_totalCursorMoves;
            ++m_totalCursorDrags;
            update();
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        if (m_draggingCursor > 0) {
            ++m_totalDragCancels;
            m_draggingCursor = 0;
            return true;
        }
        break;
    }
    default:
        break;
    }

    return false;
}

/** @brief 主题切换时重新从ThemeManager加载所有颜色成员 */
void CursorOverlay::onThemeChanged()
{
    ++m_totalThemeChanges;
    m_cursorAColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Error);
    m_cursorBColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);
    m_highlightColor = QColor(m_cursorAColor.red(), m_cursorAColor.green(),
                              m_cursorAColor.blue(), 30);
    m_textColor = ThemeManager::instance().color(ThemeManager::SemanticColor::TextPrimary);
    m_panelBgColor = ThemeManager::instance().color(ThemeManager::SemanticColor::BgSecondary);
    update();
}
