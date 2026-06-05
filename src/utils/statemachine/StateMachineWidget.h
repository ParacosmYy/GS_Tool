/**
 * @file StateMachineWidget.h
 * @brief 状态机可视化编辑控件 — 拖拽式状态图编辑器
 * @author Serial Tool Team
 * @date 2026-06-06
 *
 * 提供可视化状态机画布：状态以圆角矩形绘制，迁移以箭头连线绘制，
 * 支持拖拽移动状态、点击选中、双击编辑、右键上下文菜单。
 */

#ifndef STATEMACHINEWIDGET_H
#define STATEMACHINEWIDGET_H

#include <QMap>
#include <QWidget>
#include <QtGlobal>

#include "utils/statemachine/SmTypes.h"

class QMenu;
class QLineEdit;
class QInputDialog;

/**
 * @class StateMachineWidget
 * @brief 可视化状态机编辑画布
 */
class StateMachineWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造画布 @param parent 父控件 */
    explicit StateMachineWidget(QWidget *parent = nullptr);

    /** @brief 设置状态机数据 @param machine 状态机模型 */
    void setMachine(const SmMachine &machine);

    /** @brief 获取当前状态机数据 @return 状态机模型 */
    SmMachine machine() const;

    // ---- 统计 ----
    /** @brief 获取状态拖动次数 */
    quint64 totalStateMoves() const;
    /** @brief 获取重绘次数 */
    quint64 totalRepaints() const;
    /** @brief 获取鼠标点击次数 */
    quint64 totalMouseClicks() const;
    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 状态机内容变更信号 */
    void machineChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void drawGrid(QPainter &painter) const;
    void drawState(QPainter &painter, const SmState &st) const;
    void drawTransitions(QPainter &painter) const;
    void drawInitialIndicator(QPainter &painter, const SmState &st) const;
    void drawArrowHead(QPainter &painter, const QPointF &from,
                       const QPointF &to) const;

    QString stateAtPosition(const QPointF &pos) const;
    QRectF stateRect(const SmState &st) const;

    void addStateAt(const QPointF &pos);
    void removeSelectedState();
    void editSelectedState();
    void addTransitionDialog();
    void removeTransitionDialog();

    SmMachine m_machine;                ///< 当前状态机数据
    QString m_selectedState;            ///< 选中状态名称
    QString m_dragState;                ///< 正在拖拽的状态名称
    QPointF m_dragOffset;               ///< 拖拽偏移量
    int m_selectedTransition = -1;      ///< 选中迁移索引

    quint64 m_totalStateMoves = 0;
    quint64 m_totalRepaints = 0;
    quint64 m_totalMouseClicks = 0;
};

#endif // STATEMACHINEWIDGET_H
