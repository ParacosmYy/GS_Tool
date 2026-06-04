/**
 * @file ScatterWidget.h
 * @brief X/Y散点图控件 -- 双通道数据相关性分析面板
 *
 * 自包含QWidget，从ChartModel读取两个通道数据绘制X/Y散点图。
 * 支持X/Y轴通道选择、手动/自动刷新、Pearson相关系数计算。
 * 协作: ChartModel(数据源), ThemeManager(主题色), ChartColors(散点颜色)
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

/** @brief X/Y散点图控件，独立面板含散点图和配置工具栏，计算并显示Pearson相关系数 */
class ScatterWidget : public QWidget {
    Q_OBJECT

public:
    explicit ScatterWidget(ChartModel* model, QWidget* parent = nullptr); ///< 构造散点图控件，model为外部拥有的数据模型

public slots:
    void refreshPlot(); ///< 刷新散点图，从ChartModel读取当前X/Y通道数据并计算Pearson相关系数

    // ---- 统计计数接口 ----
    /** @brief 获取累计绘制点数 @return 绘制点计数 */
    quint64 totalPointsPlotted() const;
    /** @brief 获取累计添加点数 @return 每次刷新时新增的散点数 */
    quint64 totalPointsAdded() const;
    /** @brief 获取累计移除点数 @return 数据清空时清除的散点数 */
    quint64 totalPointsRemoved() const;
    /** @brief 获取累计自动拟合次数 @return 坐标轴范围自适应次数 */
    quint64 totalAutoFits() const;
    /** @brief 获取累计轴变更次数 @return X/Y通道切换触发次数 */
    quint64 totalAxisChanges() const;
    /** @brief 获取累计选择次数 @return 数据刷新时的散点集替换次数 */
    quint64 totalSelections() const;
    /** @brief 获取累计更新次数 @return 含手动/自动刷新的更新计数 */
    quint64 totalUpdates() const;
    /** @brief 获取累计清除次数 @return 清除计数 */
    quint64 totalClears() const;
    /** @brief 获取最近一次刷新的点密度网格最大值 @return 密度网格最大值 */
    int pointDensityMax() const;
    /** @brief 获取最近一次刷新的所有散点Y值平均 @return Y值平均值 */
    double averageValue() const;
    /** @brief 获取累计散点数据更新次数(refreshPlot中有效数据绘制) */
    quint64 totalPointUpdates() const { return m_totalPointUpdates; }
    /** @brief 获取累计坐标轴自动缩放次数 */
    quint64 totalAxisAutoScales() const { return m_totalAxisAutoScales; }
    /** @brief 获取累计渲染次数(图表重绘) */
    quint64 totalRenders() const { return m_totalRenders; }
    void resetScatterStatistics();       ///< 重置所有散点图统计计数器

private slots:
    void onXChannelChanged(int index);   ///< X轴通道选择变更
    void onYChannelChanged(int index);   ///< Y轴通道选择变更
    void onAutoRefreshToggled(bool checked); ///< 自动刷新开关切换
    void onDataUpdated(const QStringList& updatedChannels); ///< ChartModel数据更新时触发重绘
    void onChannelsChanged();            ///< 通道列表变更时更新通道选择下拉框
    void onThemeChanged();               ///< 主题切换响应

private:
    void setupUI();                      ///< 初始化UI布局
    QWidget* createToolbar();            ///< 创建顶部配置工具栏
    void setupChart();                   ///< 创建散点图图表区域
    void applyThemeColors();             ///< 应用当前主题颜色到图表
    double computePearsonCorrelation(    ///< 计算Pearson相关系数，数据不足或零方差时返回NaN
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

    // ---- 统计计数器 ----
    quint64 m_totalPointsPlotted = 0;  ///< 累计绘制点数
    quint64 m_totalPointsAdded = 0;    ///< 累计添加点数
    quint64 m_totalPointsRemoved = 0;  ///< 累计移除点数
    quint64 m_totalAutoFits = 0;       ///< 累计自动拟合次数
    quint64 m_totalAxisChanges = 0;    ///< 累计轴变更次数
    quint64 m_totalSelections = 0;     ///< 累计选择次数
    quint64 m_totalUpdates = 0;        ///< 累计更新次数
    quint64 m_totalClears = 0;         ///< 累计清除次数
    int m_pointDensityMax = 0;         ///< 最近一次刷新的点密度网格最大值
    double m_averageValue = 0.0;       ///< 最近一次刷新的所有散点Y值平均
    quint64 m_totalPointUpdates = 0;   ///< 累计散点数据更新次数(refreshPlot中有效数据绘制)
    quint64 m_totalAxisAutoScales = 0; ///< 累计坐标轴自动缩放次数
    quint64 m_totalRenders = 0;        ///< 累计渲染次数(图表重绘)
};

#endif // SCATTERWIDGET_H
