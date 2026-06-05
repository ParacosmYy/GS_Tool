/**
 * @file StateMachineWidget.cpp
 * @brief 状态机可视化编辑控件实现 — 绘制与交互
 * @author Serial Tool Team
 * @date 2026-06-06
 */

#include "utils/statemachine/StateMachineWidget.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

namespace {
constexpr int kStateWidth = 120;
constexpr int kStateHeight = 50;
constexpr int kGridSpacing = 20;
constexpr int kArrowSize = 10;
constexpr int kInitCircleRadius = 8;
} // namespace

/** @brief 构造函数 @param parent 父控件 */
StateMachineWidget::StateMachineWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("smCanvasWidget"));
    setMinimumSize(400, 300);
    setMouseTracking(true);
}

/** @brief 设置状态机数据并重绘 @param machine 状态机模型 */
void StateMachineWidget::setMachine(const SmMachine &machine)
{
    m_machine = machine;
    m_selectedState.clear();
    m_selectedTransition = -1;
    update();
    emit machineChanged();
}

/** @brief 获取当前状态机数据 @return 状态机模型 */
SmMachine StateMachineWidget::machine() const
{
    return m_machine;
}

// ---- 绘制 ----

/** @brief 重绘事件：依次绘制网格、迁移、状态 @param event 绘制事件 */
void StateMachineWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    ++m_totalRepaints;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawGrid(painter);
    drawTransitions(painter);
    for (const auto &st : m_machine.states) {
        drawState(painter, st);
    }
}

/** @brief 绘制网格背景 @param painter 画笔 */
void StateMachineWidget::drawGrid(QPainter &painter) const
{
    QPen gridPen(QColor(220, 220, 220), 1);
    painter.setPen(gridPen);
    int w = width();
    int h = height();
    for (int x = 0; x < w; x += kGridSpacing) {
        painter.drawLine(x, 0, x, h);
    }
    for (int y = 0; y < h; y += kGridSpacing) {
        painter.drawLine(0, y, w, y);
    }
}

/** @brief 绘制单个状态（圆角矩形 + 颜色编码） @param painter 画笔 @param st 状态 */
void StateMachineWidget::drawState(QPainter &painter, const SmState &st) const
{
    QRectF rect = stateRect(st);
    QColor fill;
    if (st.isInitial) {
        fill = QColor(52, 152, 219, 40);   // 蓝 — 初始
    } else if (st.isFinal) {
        fill = QColor(46, 204, 113, 40);   // 绿 — 终止
    } else {
        fill = QColor(255, 255, 255, 220); // 白 — 普通
    }
    bool isSelected = (st.name == m_selectedState);
    QPen border(isSelected ? QColor(231, 76, 60) : QColor(100, 100, 100), isSelected ? 2.5 : 1.5);

    painter.setPen(border);
    painter.setBrush(fill);
    painter.drawRoundedRect(rect, 8, 8);

    painter.setPen(QColor(40, 40, 40));
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, st.name);

    if (st.isInitial) {
        drawInitialIndicator(painter, st);
    }
    if (st.isFinal) {
        /* 终态双圆角矩形 */
        QRectF inner = rect.adjusted(4, 4, -4, -4);
        painter.setPen(QPen(QColor(46, 204, 113), 1.2, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(inner, 6, 6);
    }
}

/** @brief 绘制初始状态指示器（实心圆+箭头） @param painter 画笔 @param st 状态 */
void StateMachineWidget::drawInitialIndicator(QPainter &painter,
                                               const SmState &st) const
{
    QRectF rect = stateRect(st);
    qreal cx = rect.left() - kInitCircleRadius * 2.5;
    qreal cy = rect.center().y();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(52, 152, 219));
    painter.drawEllipse(QPointF(cx, cy), kInitCircleRadius, kInitCircleRadius);

    QPointF from(cx + kInitCircleRadius, cy);
    QPointF to(rect.left(), cy);
    QPen arrowPen(QColor(52, 152, 219), 1.5);
    painter.setPen(arrowPen);
    painter.drawLine(from, to);
    drawArrowHead(painter, from, to);
}

/** @brief 绘制所有迁移箭头和标签 @param painter 画笔 */
void StateMachineWidget::drawTransitions(QPainter &painter) const
{
    for (int i = 0; i < m_machine.transitions.size(); ++i) {
        const auto &t = m_machine.transitions.at(i);
        QRectF srcRect, tgtRect;
        bool foundSrc = false, foundTgt = false;
        for (const auto &st : m_machine.states) {
            if (st.name == t.sourceState) {
                srcRect = stateRect(st); foundSrc = true;
            }
            if (st.name == t.targetState) {
                tgtRect = stateRect(st); foundTgt = true;
            }
        }
        if (!foundSrc || !foundTgt) continue;

        QPointF from = srcRect.center();
        QPointF to = tgtRect.center();
        bool isSel = (i == m_selectedTransition);
        QPen linePen(isSel ? QColor(231, 76, 60) : QColor(120, 120, 120),
                     isSel ? 2.0 : 1.2);
        painter.setPen(linePen);
        painter.drawLine(from, to);
        drawArrowHead(painter, from, to);

        /* 标签 */
        if (!t.event.isEmpty()) {
            QPointF mid((from.x() + to.x()) / 2, (from.y() + to.y()) / 2 - 10);
            QFont f = painter.font();
            f.setPointSize(8);
            f.setItalic(true);
            painter.setFont(f);
            painter.setPen(QColor(60, 60, 60));
            QString label = t.event;
            if (!t.guard.isEmpty()) {
                label += QStringLiteral("[%1]").arg(t.guard);
            }
            painter.drawText(mid, label);
        }
    }
}

/** @brief 绘制箭头头部 @param painter 画笔 @param from 起点 @param to 终点 */
void StateMachineWidget::drawArrowHead(QPainter &painter, const QPointF &from,
                                        const QPointF &to) const
{
    double angle = std::atan2(to.y() - from.y(), to.x() - from.x());
    QPointF p1(to.x() - kArrowSize * std::cos(angle - M_PI / 6),
               to.y() - kArrowSize * std::sin(angle - M_PI / 6));
    QPointF p2(to.x() - kArrowSize * std::cos(angle + M_PI / 6),
               to.y() - kArrowSize * std::sin(angle + M_PI / 6));
    QPen cur = painter.pen();
    painter.setBrush(cur.color());
    QPainterPath path;
    path.moveTo(to);
    path.lineTo(p1);
    path.lineTo(p2);
    path.closeSubpath();
    painter.drawPath(path);
    painter.setBrush(Qt::NoBrush);
}

// ---- 交互 ----

/** @brief 鼠标按下：选中状态或开始拖拽 @param event 鼠标事件 */
void StateMachineWidget::mousePressEvent(QMouseEvent *event)
{
    ++m_totalMouseClicks;
    if (event->button() != Qt::LeftButton) return;
    QString hit = stateAtPosition(event->position());
    m_selectedState = hit;
    m_selectedTransition = -1;
    if (!hit.isEmpty()) {
        m_dragState = hit;
        SmState st;
        for (const auto &s : m_machine.states) {
            if (s.name == hit) { st = s; break; }
        }
        QRectF r = stateRect(st);
        m_dragOffset = event->position() - r.topLeft();
    } else {
        m_dragState.clear();
    }
    update();
}

/** @brief 鼠标移动：拖拽状态 @param event 鼠标事件 */
void StateMachineWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragState.isEmpty()) return;
    if (!(event->buttons() & Qt::LeftButton)) return;
    for (int i = 0; i < m_machine.states.size(); ++i) {
        if (m_machine.states.at(i).name == m_dragState) {
            m_machine.states[i].position = event->position() - m_dragOffset;
            ++m_totalStateMoves;
            update();
            break;
        }
    }
}

/** @brief 鼠标释放：结束拖拽 @param event 鼠标事件 */
void StateMachineWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (!m_dragState.isEmpty()) {
        emit machineChanged();
    }
    m_dragState.clear();
}

/** @brief 双击编辑状态名称 @param event 鼠标事件 */
void StateMachineWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QString hit = stateAtPosition(event->position());
    if (hit.isEmpty()) return;
    editSelectedState();
}

/** @brief 右键上下文菜单 @param event 上下文菜单事件 */
void StateMachineWidget::contextMenuEvent(QContextMenuEvent *event)
{
    auto *menu = new QMenu(this);
    menu->setObjectName(QStringLiteral("smContextMenu"));
    menu->addAction(tr("添加状态"), this, [this, pos = event->pos()]() {
        addStateAt(pos);
    });
    if (!m_selectedState.isEmpty()) {
        menu->addAction(tr("编辑状态"), this, &StateMachineWidget::editSelectedState);
        menu->addAction(tr("删除状态"), this, &StateMachineWidget::removeSelectedState);
    }
    menu->addSeparator();
    menu->addAction(tr("添加迁移"), this, &StateMachineWidget::addTransitionDialog);
    if (m_selectedTransition >= 0) {
        menu->addAction(tr("删除迁移"), this,
                        &StateMachineWidget::removeTransitionDialog);
    }
    menu->exec(event->globalPos());
    menu->deleteLater();
}

// ---- 辅助 ----

/** @brief 计算状态的绘制矩形 @param st 状态 @return 矩形区域 */
QRectF StateMachineWidget::stateRect(const SmState &st) const
{
    return QRectF(st.position.x(), st.position.y(), kStateWidth, kStateHeight);
}

/** @brief 命中测试：返回坐标处的状态名称 @param pos 坐标 @return 状态名称或空 */
QString StateMachineWidget::stateAtPosition(const QPointF &pos) const
{
    for (const auto &st : m_machine.states) {
        if (stateRect(st).contains(pos)) return st.name;
    }
    return QString();
}

/** @brief 在指定位置添加新状态 @param pos 画布坐标 */
void StateMachineWidget::addStateAt(const QPointF &pos)
{
    bool ok = false;
    QString name = QInputDialog::getText(this, tr("添加状态"),
                                          tr("状态名称:"), QLineEdit::Normal,
                                          QString(), &ok);
    if (!ok || name.isEmpty()) return;
    for (const auto &s : m_machine.states) {
        if (s.name == name) {
            QMessageBox::warning(this, tr("错误"), tr("状态名称已存在"));
            return;
        }
    }
    SmState st;
    st.name = name;
    st.position = pos - QPointF(kStateWidth / 2.0, kStateHeight / 2.0);
    if (m_machine.states.isEmpty()) {
        st.isInitial = true;
        m_machine.initialState = name;
    }
    m_machine.states.append(st);
    m_selectedState = name;
    update();
    emit machineChanged();
}

/** @brief 移除选中状态 */
void StateMachineWidget::removeSelectedState()
{
    if (m_selectedState.isEmpty()) return;
    for (int i = 0; i < m_machine.states.size(); ++i) {
        if (m_machine.states.at(i).name == m_selectedState) {
            m_machine.states.removeAt(i);
            break;
        }
    }
    for (int j = m_machine.transitions.size() - 1; j >= 0; --j) {
        const auto &t = m_machine.transitions.at(j);
        if (t.sourceState == m_selectedState || t.targetState == m_selectedState) {
            m_machine.transitions.removeAt(j);
        }
    }
    if (m_machine.initialState == m_selectedState) {
        m_machine.initialState.clear();
    }
    m_selectedState.clear();
    update();
    emit machineChanged();
}

/** @brief 编辑选中状态的名称 */
void StateMachineWidget::editSelectedState()
{
    if (m_selectedState.isEmpty()) return;
    bool ok = false;
    QString newName = QInputDialog::getText(
        this, tr("编辑状态"), tr("状态名称:"),
        QLineEdit::Normal, m_selectedState, &ok);
    if (!ok || newName.isEmpty() || newName == m_selectedState) return;
    for (const auto &s : m_machine.states) {
        if (s.name == newName) {
            QMessageBox::warning(this, tr("错误"), tr("状态名称已存在"));
            return;
        }
    }
    for (int i = 0; i < m_machine.states.size(); ++i) {
        if (m_machine.states.at(i).name == m_selectedState) {
            m_machine.states[i].name = newName;
            break;
        }
    }
    for (auto &t : m_machine.transitions) {
        if (t.sourceState == m_selectedState) t.sourceState = newName;
        if (t.targetState == m_selectedState) t.targetState = newName;
    }
    if (m_machine.initialState == m_selectedState) {
        m_machine.initialState = newName;
    }
    m_selectedState = newName;
    update();
    emit machineChanged();
}

/** @brief 弹出添加迁移对话框 */
void StateMachineWidget::addTransitionDialog()
{
    if (m_machine.states.size() < 2) {
        QMessageBox::information(this, tr("提示"), tr("至少需要两个状态"));
        return;
    }
    QStringList names;
    for (const auto &s : m_machine.states) names << s.name;
    bool ok = false;
    QString src = QInputDialog::getItem(this, tr("添加迁移"),
                                         tr("源状态:"), names, 0, false, &ok);
    if (!ok) return;
    QString tgt = QInputDialog::getItem(this, tr("添加迁移"),
                                         tr("目标状态:"), names, 0, false, &ok);
    if (!ok) return;
    QString evt = QInputDialog::getText(this, tr("添加迁移"),
                                         tr("事件名称:"), QLineEdit::Normal,
                                         QString(), &ok);
    if (!ok || evt.isEmpty()) return;
    SmTransition trans;
    trans.sourceState = src;
    trans.targetState = tgt;
    trans.event = evt;
    m_machine.transitions.append(trans);
    m_selectedTransition = m_machine.transitions.size() - 1;
    update();
    emit machineChanged();
}

/** @brief 删除选中的迁移 */
void StateMachineWidget::removeTransitionDialog()
{
    if (m_selectedTransition < 0
        || m_selectedTransition >= m_machine.transitions.size()) {
        return;
    }
    m_machine.transitions.removeAt(m_selectedTransition);
    m_selectedTransition = -1;
    update();
    emit machineChanged();
}
