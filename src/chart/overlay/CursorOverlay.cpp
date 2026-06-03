/**
 * @file CursorOverlay.cpp
 * @brief 波形游标测量叠加层实现
 *
 * 实现双游标竖线绘制、选中区域高亮和差值信息面板。
 * 游标A(红色)通过双击放置，游标B(蓝色)通过右键放置。
 * 支持鼠标拖拽移动游标位置。
 *
 * 事件处理策略:
 *   CursorOverlay自身设置WA_TransparentForMouseEvents，不接收鼠标事件。
 *   通过eventFilter安装在QChartView上（优先级高于ZoomController），
 *   命中游标时消费事件，未命中时放行给ZoomController。
 */

#include "chart/overlay/CursorOverlay.h"
#include "chart/model/ChartModel.h"
#include "chart/zoom/ZoomController.h"
#include "core/theme/ThemeManager.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QWidget>
#include <QChartView>
#include <QChart>
#include <QValueAxis>
#include <algorithm>
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
// 公开接口
// ============================================================

/** @brief 设置游标A的X轴坐标值(数据空间) @param x X轴值 */
void CursorOverlay::setCursorA(double x)
{
    m_cursorAX = x;
    m_hasCursorA = true;
    ++m_totalCursorMoves;
    update();
}

/** @brief 设置游标B的X轴坐标值(数据空间) @param x X轴值 */
void CursorOverlay::setCursorB(double x)
{
    m_cursorBX = x;
    m_hasCursorB = true;
    ++m_totalCursorMoves;
    update();
}

/** @brief 清除所有游标 */
void CursorOverlay::clearCursors()
{
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

    // 游标未激活时不拦截任何事件
    if (!isVisible()) return false;

    switch (event->type()) {
    case QEvent::MouseButtonDblClick: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            setCursorA(pixelToDataX(static_cast<int>(me->position().x())));
            return true;  // 消费：双击放置游标A
        }
        break;
    }
    case QEvent::MouseButtonPress: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::RightButton) {
            setCursorB(pixelToDataX(static_cast<int>(me->position().x())));
            return true;  // 消费：右键放置游标B
        }
        if (me->button() == Qt::LeftButton) {
            int hit = hitTestCursor(static_cast<int>(me->position().x()));
            if (hit > 0) {
                m_draggingCursor = hit;
                return true;  // 消费：开始拖拽游标
            }
            // 未命中游标 → 放行给ZoomController
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
            update();
            return true;  // 消费：拖拽中
        }
        // 非拖拽 → 放行
        break;
    }
    case QEvent::MouseButtonRelease: {
        if (m_draggingCursor > 0) {
            m_draggingCursor = 0;
            return true;  // 消费：结束拖拽
        }
        break;
    }
    default:
        break;
    }

    return false;  // 未消费 → 放行给ZoomController
}

// ============================================================
// 绘制
// ============================================================

/** @brief 绘制单条游标竖线 @param painter 画笔 @param pixelX 游标像素X位置 @param color 游标颜色 @param label 游标标签(A/B) */
void CursorOverlay::drawCursorLine(QPainter& painter, double pixelX,
                                   const QColor& color, const QString& label)
{
    painter.setPen(QPen(color, 2, Qt::DashLine));
    painter.drawLine(static_cast<int>(pixelX), 0,
                     static_cast<int>(pixelX), height());

    // 绘制标签
    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    painter.setPen(color);

    QRect textRect(static_cast<int>(pixelX) + 4, 4, 20, 16);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, label);
}

/** @brief 绘制游标间的高亮区域 @param painter 画笔 @param pixelAX 游标A像素X @param pixelBX 游标B像素X */
void CursorOverlay::drawHighlightRegion(QPainter& painter, double pixelAX,
                                        double pixelBX)
{
    int left = static_cast<int>(std::min(pixelAX, pixelBX));
    int right = static_cast<int>(std::max(pixelAX, pixelBX));
    painter.fillRect(left, 0, right - left, height(), m_highlightColor);
}

/**
 * @brief 在已排序数据中用二分查找定位游标X对应的Y值(含线性插值)
 * @param data 已按X排序的数据点
 * @param x 目标X值
 * @param outY 输出Y值
 * @return true=找到并插值成功
 */
    static bool interpolateY(const QVector<QPointF>& data, double x, double& outY)
    {
        if (data.isEmpty()) return false;

        // X在数据范围外 — 取边界值
        if (x <= data.first().x()) { outY = data.first().y(); return true; }
        if (x >= data.last().x()) { outY = data.last().y(); return true; }

        // 二分查找第一个 x >= target 的位置
        auto it = std::lower_bound(data.begin(), data.end(), x,
            [](const QPointF& pt, double val) { return pt.x() < val; });

        if (it == data.end()) { outY = data.last().y(); return true; }
        if (it == data.begin()) { outY = it->y(); return true; }

        // 线性插值: it-1 和 it 之间
        const QPointF& p0 = *(it - 1);
        const QPointF& p1 = *it;
        double dx = p1.x() - p0.x();
        if (qFuzzyIsNull(dx)) {
            // 相邻数据点X值相同，取平均值
            outY = (p0.y() + p1.y()) * 0.5;
            return true;
        }
        double t = (x - p0.x()) / dx;
        outY = p0.y() + t * (p1.y() - p0.y());
        return true;
    }

/**
 * @brief 绘制差值信息面板
 * @param painter 画笔
 *
 * 在图表右上角绘制一个半透明面板，显示:
 *   deltaX (采样点差)
 *   1/deltaX (频率估算)
 *   各通道的 deltaY (值差)
 */
void CursorOverlay::drawDeltaPanel(QPainter& painter)
{
    if (!m_hasCursorA || !m_hasCursorB || !m_model) return;

    double deltaX = std::abs(m_cursorBX - m_cursorAX);

    // 构建差值文本(使用tr()包裹用户可见文字)
    QStringList lines;
    lines << tr("ΔX: %1 采样").arg(deltaX, 0, 'f', 1);
    if (deltaX > 0) {
        lines << tr("1/ΔX: %1").arg(1.0 / deltaX, 0, 'f', 4);
    }

    // 各通道ΔY — 使用二分查找+插值(避免O(N)遍历和QVector拷贝)
    for (const QString& ch : m_model->channelNames()) {
        const QVector<QPointF>& data = m_model->channelData(ch);
        double valA = 0.0, valB = 0.0;
        if (interpolateY(data, m_cursorAX, valA) &&
            interpolateY(data, m_cursorBX, valB)) {
            lines << tr("%1 ΔY: %2").arg(ch).arg(valB - valA, 0, 'f', 3);
        }
    }

    // 绘制面板背景
    QFont font = painter.font();
    font.setPixelSize(11);
    painter.setFont(font);

    QFontMetrics fm(font);
    int maxW = 0;
    for (const QString& line : lines) {
        maxW = std::max(maxW, fm.horizontalAdvance(line));
    }
    int panelW = maxW + 16;
    int panelH = fm.height() * lines.size() + 12;
    int panelX = width() - panelW - 10;
    int panelY = 10;

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_panelBgColor);
    painter.drawRoundedRect(panelX, panelY, panelW, panelH, 4, 4);

    // 绘制文本
    painter.setPen(m_textColor);
    int textY = panelY + 6;
    for (const QString& line : lines) {
        painter.drawText(panelX + 8, textY + fm.ascent(), line);
        textY += fm.height();
    }
}

/**
 * @brief 绘制游标线和差值信息面板
 * @param event 绘制事件(未使用)
 *
 * 绘制顺序: 高亮区域 → 游标A线 → 游标B线 → 差值面板
 */
void CursorOverlay::paintEvent(QPaintEvent* /*event*/)
{
    if (!m_hasCursorA && !m_hasCursorB) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 双游标模式: 绘制高亮区域
    if (m_hasCursorA && m_hasCursorB) {
        double pixelAX = dataToPixelX(m_cursorAX);
        double pixelBX = dataToPixelX(m_cursorBX);
        drawHighlightRegion(painter, pixelAX, pixelBX);
    }

    // 绘制游标A
    if (m_hasCursorA) {
        drawCursorLine(painter, dataToPixelX(m_cursorAX),
                       m_cursorAColor, "A");
    }

    // 绘制游标B
    if (m_hasCursorB) {
        drawCursorLine(painter, dataToPixelX(m_cursorBX),
                       m_cursorBColor, "B");
    }

    // 双游标模式: 绘制差值面板
    if (m_hasCursorA && m_hasCursorB) {
        ++m_totalMeasurements;
        drawDeltaPanel(painter);
    }

    // 绘制框选缩放的橡皮筋矩形(来自ZoomController)
    if (m_zoomController && m_zoomController->isRubberBandActive()) {
        QRectF rubberRect = m_zoomController->rubberBandRect();
        painter.setPen(QPen(m_cursorAColor, 1, Qt::DashLine));
        // 使用主题Accent色半透明填充，在深色和浅色主题下均可见
        QColor rubberFill = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);
        rubberFill.setAlpha(30);
        painter.setBrush(rubberFill);
        painter.drawRect(rubberRect);
    }
}

// ============================================================
// 主题切换
// ============================================================

/** @brief 主题切换时重新从ThemeManager加载所有颜色成员 */
void CursorOverlay::onThemeChanged()
{
    m_cursorAColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Error);
    m_cursorBColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);
    m_highlightColor = QColor(m_cursorAColor.red(), m_cursorAColor.green(),
                              m_cursorAColor.blue(), 30);
    m_textColor = ThemeManager::instance().color(ThemeManager::SemanticColor::TextPrimary);
    m_panelBgColor = ThemeManager::instance().color(ThemeManager::SemanticColor::BgSecondary);
    update();
}

// ============================================================
// 统计计数器接口
// ============================================================

/** @brief 返回游标移动总次数（含放置和拖拽） @return 移动总次数 */
quint64 CursorOverlay::totalCursorMoves() const
{
    return m_totalCursorMoves;
}

/** @brief 返回测量显示总次数（双游标差值面板绘制） @return 测量总次数 */
quint64 CursorOverlay::totalMeasurements() const
{
    return m_totalMeasurements;
}

/** @brief 重置所有游标统计计数器为初始值 */
void CursorOverlay::resetCursorStatistics()
{
    m_totalCursorMoves = 0;
    m_totalMeasurements = 0;
}
