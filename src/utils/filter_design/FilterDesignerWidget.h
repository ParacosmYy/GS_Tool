/**
 * @file FilterDesignerWidget.h
 * @brief 数字滤波器可视化设计控件 — 参数面板 + 频率响应绘制
 *
 * 设计: QWidget子类、左面板参数控件 + 右面板QPainter绘制频响曲线
 * 协作: FilterDesigner(设计引擎) / FilterTypes(数据结构)
 *
 * 功能:
 *   - 滤波器类型/族/阶数/截止频率/窗函数等参数控件
 *   - 自定义paintEvent绘制幅度(dB)频率响应曲线 + 网格
 *   - 相位响应叠加显示开关
 *   - 参数变更自动触发重新设计
 */

#ifndef FILTERDESIGNERWIDGET_H
#define FILTERDESIGNERWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>

#include "utils/filter_design/FilterTypes.h"

class FilterDesigner;
class QPaintEvent;
class QSplitter;

/** @brief 数字滤波器可视化设计控件 — 参数面板 + 频率响应绘制 */
class FilterDesignerWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造滤波器设计控件 @param parent 父控件 */
    explicit FilterDesignerWidget(QWidget* parent = nullptr);

    // ---- 外部接口 ----

    /** @brief 绑定滤波器设计引擎(不获取所有权) @param designer 引擎指针 */
    void setDesigner(FilterDesigner* designer);

    /** @brief 获取当前滤波器参数 @return 参数集 */
    FilterParams currentParams() const;

    /** @brief 获取当前滤波器系数 @return 系数 */
    FilterCoeffs currentCoeffs() const;

    /** @brief 获取当前频率响应 @return 响应数据 */
    FilterResponse currentResponse() const;

    // ---- 统计 ----

    /** @brief 获取累计重绘次数 */
    quint64 totalRepaints() const;

    /** @brief 获取累计参数变更次数 */
    quint64 totalParamChanges() const;

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 滤波器参数变更信号(参数变化后发射) */
    void filterChanged();

protected:
    /** @brief 自定义绘制事件 — 渲染频率响应曲线+网格+标注 */
    void paintEvent(QPaintEvent* event) override;

private slots:
    /** @brief 参数控件值变更时触发重新设计 */
    void onParameterChanged();

private:
    // ---- UI 构建 ----

    /** @brief 创建左侧参数面板 @return 面板Widget */
    QWidget* createParameterPanel();

    /** @brief 创建右侧绘图区域Widget */
    QWidget* createPlotArea();

    // ---- 绘制 ----

    /** @brief 绘制背景网格 @param painter 画笔 @param plotRect 绘图区域 */
    void drawGrid(QPainter& painter, const QRectF& plotRect) const;

    /** @brief 绘制幅度响应曲线 @param painter 画笔 @param plotRect 绘图区域 */
    void drawMagnitudeCurve(QPainter& painter, const QRectF& plotRect) const;

    /** @brief 绘制相位响应曲线(虚线) @param painter 画笔 @param plotRect 绘图区域 */
    void drawPhaseCurve(QPainter& painter, const QRectF& plotRect) const;

    /** @brief 绘制坐标轴标签 @param painter 画笔 @param plotRect 绘图区域 */
    void drawAxisLabels(QPainter& painter, const QRectF& plotRect) const;

    // ---- 内部逻辑 ----

    /** @brief 根据当前参数重新设计滤波器并计算频响 */
    void redesignFilter();

    // ---- 引擎 ----
    FilterDesigner* m_designer = nullptr;   ///< 设计引擎(外部拥有)

    // ---- 参数控件 ----
    QComboBox*      m_typeCombo   = nullptr; ///< 滤波器类型
    QComboBox*      m_familyCombo = nullptr; ///< 滤波器族
    QSpinBox*       m_orderSpin   = nullptr; ///< 滤波器阶数
    QDoubleSpinBox* m_cutoffSpin  = nullptr; ///< 截止频率
    QDoubleSpinBox* m_sampleSpin  = nullptr; ///< 采样率
    QComboBox*      m_windowCombo = nullptr; ///< FIR窗类型
    QCheckBox*      m_phaseCheck  = nullptr; ///< 相位叠加开关

    // ---- 当前结果 ----
    FilterParams   m_params;                ///< 当前参数
    FilterCoeffs   m_coeffs;                ///< 当前系数
    FilterResponse m_response;              ///< 当前频响

    // ---- 统计 ----
    quint64 m_totalRepaints     = 0;        ///< 累计重绘次数
    quint64 m_totalParamChanges = 0;        ///< 累计参数变更次数
};

#endif // FILTERDESIGNERWIDGET_H
