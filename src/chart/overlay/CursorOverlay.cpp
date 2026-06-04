/**
 * @file CursorOverlay.cpp
 * @brief 波形游标测量叠加层 -- 构造、游标管理、事件处理实现
 *
 * 本文件实现 CursorOverlay 的非绘制部分:
 *   - 构造/析构: 初始化颜色、信号连接
 *   - 游标管理: setCursorA/B, clearCursors, 查询接口
 *   - 坐标映射: pixelToDataX / dataToPixelX
 *   - 命中检测: hitTestCursor
 *   - 事件过滤: 双击/右键放置、左键拖拽
 *   - 主题切换: onThemeChanged
 *   - 统计计数器: totalCursorMoves/totalMeasurements/resetCursorStatistics
 *
 * 绘制相关方法见 CursorOverlayPaint.cpp。
 * 统计计数器接口见 CursorOverlayInteraction.cpp。
 *
 * 事件处理策略:
 *   CursorOverlay自身设置WA_TransparentForMouseEvents，不接收鼠标事件。
 *   通过eventFilter安装在QChartView上（优先级高于ZoomController），
 *   命中游标时消费事件，未命中时放行给ZoomController。
 */

#include "chart/overlay/CursorOverlay.h"
#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

#include <QMouseEvent>
#include <QShowEvent>
#include <QWidget>
#include <QChartView>
#include <QChart>
#include <cmath>

// ============================================================
// 构造
// ============================================================

/** @brief 构造游标叠加层 @param chartView 关联的图表视图(用于坐标映射) @param model 数据模型(用于读取Y值) @param parent 父控件 */
CursorOverlay::CursorOverlay(QChartView* chartView, ChartModel* model,
                             QWidget* parent)
    : QWidget(parent)
    , m_chartView(chartView)
    , m_model(model)
{
    setObjectName("cursorOverlay");

    // 设置为透明背景，鼠标事件穿透到QChartView
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    // 从语义色板初始化颜色
    m_cursorAColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Error);
    m_cursorBColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);
    m_highlightColor = QColor(m_cursorAColor.red(), m_cursorAColor.green(),
                              m_cursorAColor.blue(), 30);
    m_textColor = ThemeManager::instance().color(ThemeManager::SemanticColor::TextPrimary);
    m_panelBgColor = ThemeManager::instance().color(ThemeManager::SemanticColor::BgSecondary);

    // 主题切换时刷新颜色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &CursorOverlay::onThemeChanged);
}

// ============================================================
// 显示事件
// ============================================================

/** @brief 首次显示时同步几何尺寸到父控件(chartView) @param event 显示事件 */
void CursorOverlay::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // 首次显示时chartView布局已计算完成，同步overlay尺寸
    if (m_chartView) {
        setGeometry(m_chartView->rect());
    }
}

// ============================================================
// 公开接口 — 游标管理
// ============================================================

/** @brief 设置游标A的X轴坐标值(数据空间) @param x X轴值 */
void CursorOverlay::setCursorA(double x)
{
    m_cursorAX = x;
    m_hasCursorA = true;
    ++m_totalCursorMoves;
    ++m_totalCursorCreations;
    update();
}

/** @brief 设置游标B的X轴坐标值(数据空间) @param x X轴值 */
void CursorOverlay::setCursorB(double x)
{
    m_cursorBX = x;
    m_hasCursorB = true;
    ++m_totalCursorMoves;
    ++m_totalCursorCreations;
    update();
}

/** @brief 清除所有游标 */
void CursorOverlay::clearCursors()
{
    if (m_hasCursorA || m_hasCursorB) {
        ++m_totalCursorDeletions;
    }
    m_hasCursorA = false;
    m_hasCursorB = false;
    update();
}

/** @brief 查询游标A是否已放置 @return true=已放置 */
bool CursorOverlay::hasCursorA() const { return m_hasCursorA; }

/** @brief 查询游标B是否已放置 @return true=已放置 */
bool CursorOverlay::hasCursorB() const { return m_hasCursorB; }

/** @brief 获取游标A的X轴坐标值(数据空间) @return 游标A的X值 */
double CursorOverlay::cursorAX() const { return m_cursorAX; }

/** @brief 获取游标B的X轴坐标值(数据空间) @return 游标B的X值 */
double CursorOverlay::cursorBX() const { return m_cursorBX; }

/** @brief 设置关联的缩放控制器(用于绘制框选矩形) @param zoom 缩放控制器指针 */
void CursorOverlay::setZoomController(ZoomController* zoom)
{
    m_zoomController = zoom;
}

// ============================================================
// 坐标映射
// ============================================================

/** @brief 从像素X坐标转换为数据空间X值 @param pixelX 像素坐标(相对于QChartView) @return 数据空间X值 */
double CursorOverlay::pixelToDataX(int pixelX) const
{
    if (!m_chartView || !m_chartView->chart()) return 0.0;
    QPointF dataPt = m_chartView->chart()->mapToValue(
        QPointF(pixelX, 0));
    return dataPt.x();
}

/** @brief 从数据空间X值转换为像素X坐标 @param dataX 数据空间X值 @return 像素坐标(相对于QChartView) */
double CursorOverlay::dataToPixelX(double dataX) const
{
    if (!m_chartView || !m_chartView->chart()) return 0.0;
    QPointF pixelPt = m_chartView->chart()->mapToPosition(
        QPointF(dataX, 0));
    return pixelPt.x();
}

// ============================================================
// 命中检测
// ============================================================

/** @brief 判断点击位置是否在游标附近(可拖拽) @param pixelX 点击像素X @return 0=无, 1=游标A, 2=游标B */
int CursorOverlay::hitTestCursor(int pixelX) const
{
    if (m_hasCursorA) {
        double aPixel = dataToPixelX(m_cursorAX);
        if (std::abs(pixelX - aPixel) <= kHitMargin) return 1;
    }
    if (m_hasCursorB) {
        double bPixel = dataToPixelX(m_cursorBX);
        if (std::abs(pixelX - bPixel) <= kHitMargin) return 2;
    }
    return 0;
}

// ============================================================
// 事件过滤 — 安装在QChartView上，优先级高于ZoomController
// ============================================================

// eventFilter/onThemeChanged见 CursorOverlayEvent.cpp

// 统计计数器接口(totalCursorCreations/totalCursorDeletions/totalCursorDrags/
// totalCursorMoves/totalDeltaMeasurements/totalMeasurements/totalSnapToPeak/
// averageDeltaX/averageDeltaY/resetCursorStatistics)
// 见 CursorOverlayInteraction.cpp
