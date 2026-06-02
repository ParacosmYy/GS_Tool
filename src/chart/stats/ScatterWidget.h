/**
 * @file ScatterWidget.h
 * @brief X/Y散点图控件 -- 双通道数据相关性分析面板
 *
 * 设计要点:
 *   1. 自包含QWidget，可直接作为标签页或停靠窗口添加
 *   2. 从ChartModel读取两个通道的数据，绘制X/Y散点图
 *   3. 支持X轴通道选择、Y轴通道选择、手动/自动刷新
 *   4. 计算并显示Pearson相关系数，评估两通道线性相关性
 *   5. 主题感知: 背景色、网格线、标签色跟随ThemeManager
 *
 * 协作关系:
 *   - ChartModel: 提供通道数据（channelData/channelNames方法）
 *   - ThemeManager: 监听themeChanged信号更新视觉样式
 *   - ChartColors: 散点颜色使用主题调色板第一色
 */

#ifndef SCATTERWIDGET_H
#define SCATTERWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QVector>
#include <QPointF>

#include "chart/widget/ChartColors.h"

class QChartView;
class QChart;
class QScatterSeries;
class QValueAxis;
class QComboBox;
class QPushButton;
class QCheckBox;
class QLabel;
class ChartModel;

/**
 * @brief X/Y散点图控件
 *
 * 独立面板控件，内含散点图和配置工具栏。
 * 从ChartModel读取两个指定通道的数据，绘制X-Y散点图，
 * 计算并显示Pearson相关系数。
 *
 * 功能:
 *   - X轴通道选择: 下拉框选择X轴数据源通道
 *   - Y轴通道选择: 下拉框选择Y轴数据源通道
 *   - 手动刷新按钮 + 自动刷新复选框
 *   - Pearson相关系数实时显示
 */
class ScatterWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造散点图控件
     * @param model 数据模型指针（外部拥有，不负责销毁）
     * @param parent 父控件
     */
    explicit ScatterWidget(ChartModel* model, QWidget* parent = nullptr);

public slots:
    /**
     * @brief 刷新散点图显示
     *
     * 从ChartModel读取当前选中X/Y通道的数据，
     * 绘制散点图并计算Pearson相关系数。
     */
    void refreshPlot();

private slots:
    /** @brief X轴通道选择变更 */
    void onXChannelChanged(int index);

    /** @brief Y轴通道选择变更 */
    void onYChannelChanged(int index);

    /** @brief 自动刷新开关切换 */
    void onAutoRefreshToggled(bool checked);

    /** @brief ChartModel数据更新时（自动刷新模式下触发重绘） */
    void onDataUpdated(const QStringList& updatedChannels);

    /** @brief 通道列表变更时更新通道选择下拉框 */
    void onChannelsChanged();

    /** @brief 主题切换响应 -- 更新图表所有视觉元素 */
    void onThemeChanged();

private:
    /** @brief 初始化UI布局 */
    void setupUI();

    /** @brief 创建顶部配置工具栏 */
    QWidget* createToolbar();

    /** @brief 创建散点图图表区域 */
    void setupChart();

    /** @brief 应用当前主题颜色到图表 */
    void applyThemeColors();

    /**
     * @brief 计算Pearson相关系数
     *
     * 标准Pearson r公式:
     *   r = Σ((x_i - x̄)(y_i - ȳ)) / sqrt(Σ(x_i - x̄)² × Σ(y_i - ȳ)²)
     *
     * @param xData X轴数据（取QPointF的y分量作为值）
     * @param yData Y轴数据（取QPointF的y分量作为值）
     * @return Pearson相关系数，数据不足或零方差时返回NaN
     */
    double computePearsonCorrelation(
        const QVector<QPointF>& xData, const QVector<QPointF>& yData);

    ChartModel* m_model;              ///< 数据模型（外部拥有）

    // ---- 图表组件 ----
    QChartView* m_chartView;          ///< 图表视图
    QChart* m_chart;                  ///< Qt Charts 图表对象
    QScatterSeries* m_series;         ///< 散点序列
    QValueAxis* m_xAxis;             ///< X轴
    QValueAxis* m_yAxis;             ///< Y轴

    // ---- 工具栏控件 ----
    QComboBox* m_xChannelCombo;      ///< X轴通道选择下拉框
    QComboBox* m_yChannelCombo;      ///< Y轴通道选择下拉框
    QPushButton* m_refreshBtn;       ///< 手动刷新按钮
    QCheckBox* m_autoRefreshCheck;   ///< 自动刷新复选框
    QLabel* m_correlationLabel;      ///< 相关系数标签（Pearson r）

    // ---- 状态 ----
    bool m_autoRefresh = false;      ///< 是否自动刷新
};

#endif // SCATTERWIDGET_H
