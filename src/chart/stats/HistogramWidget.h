/**
 * @file HistogramWidget.h
 * @brief 统计直方图控件 -- 通道数据值分布分析面板
 *
 * 自包含QWidget, 从ChartModel读取通道数据计算值分布直方图并渲染。
 * 支持通道选择、分桶数配置、手动/自动刷新、统计摘要(均值/标准差/min/max/样本数)。
 * 协作: ChartModel(数据) / ThemeManager(主题) / ChartColors(柱体颜色)
 */

#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QVector>
#include <QPointF>
#include <QPair>

#include "chart/widget/ChartColors.h"

class QChartView; class QChart; class QBarSeries; class QBarSet;
class QBarCategoryAxis; class QValueAxis;
class QComboBox; class QSpinBox; class QPushButton; class QCheckBox; class QLabel;
class ChartModel;

/** @brief 统计直方图控件 -- 独立面板，内含直方图图表和配置工具栏 */
class HistogramWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 直方图统计摘要数据结构 */
    struct Stats { double mean; double stddev; double min; double max; int count; };

    /** @brief 构造直方图控件 @param model 外部拥有的ChartModel数据模型 @param parent 父控件指针 */
    explicit HistogramWidget(ChartModel* model, QWidget* parent = nullptr);
    /** @brief 计算直方图分桶 @param data 输入数据点集 @param bins 分桶数量 @return (桶中心值, 计数)对列表 */
    static QVector<QPair<double, int>> computeHistogram(const QVector<QPointF>& data, int bins);
    /** @brief 计算统计摘要 @param data 输入数据点集 @return 统计摘要(均值/标准差/最小/最大/样本数) */
    static Stats computeStats(const QVector<QPointF>& data);

public slots:
    void refreshHistogram();                              ///< 刷新直方图(从ChartModel读取数据重算)
    bool exportToCsv(const QString& filePath);            ///< 导出当前直方图数据到CSV

    // ---- 统计计数接口 ----
    /** @brief 获取累计刷新更新次数 @return 更新计数 */
    quint64 totalUpdates() const;
    /** @brief 获取累计分桶数变更次数 @return 变更计数 */
    quint64 totalBinChanges() const;
    /** @brief 获取累计分桶计算次数 @return 计算计数 */
    quint64 totalBinsComputed() const;
    /** @brief 获取峰值所在桶索引 @return 索引号 */
    int peakBinIndex() const;
    /** @brief 获取最大桶计数值 @return 最大计数 */
    int maxBinCount() const;
    /** @brief 获取累计分桶重计算次数 @return 重算计数 */
    quint64 totalBinRecalculations() const;
    /** @brief 获取累计分布更新次数 @return 更新计数 */
    quint64 totalDistributionUpdates() const;
    /** @brief 获取累计Y轴自动范围调整次数 @return 调整计数 */
    quint64 totalAutoRanges() const;
    /** @brief 获取累计导出次数 @return 导出计数 */
    quint64 totalExports() const;
    /** @brief 获取累计通道切换次数 @return 切换计数 */
    quint64 totalChannelSwitches() const;
    /** @brief 获取累计自动刷新开关切换次数 @return 切换计数 */
    quint64 totalAutoRefreshToggles() const;
    /** @brief 获取累计主题颜色变更次数 @return 变更计数 */
    quint64 totalThemeChanges() const;
    /** @brief 重置所有直方图统计计数器 */
    void resetHistogramStatistics();

private slots:
    /** @brief 通道选择变更处理 @param index 下拉框选中索引 */
    void onChannelChanged(int index);
    /** @brief 分桶数变更处理 @param value 新的分桶数 */
    void onBinsChanged(int value);
    /** @brief 自动刷新开关切换 @param checked 是否启用 */
    void onAutoRefreshToggled(bool checked);
    /** @brief ChartModel数据更新时触发重绘 @param updatedChannels 已更新的通道列表 */
    void onDataUpdated(const QStringList& updatedChannels);
    /** @brief 通道列表变更时更新下拉框 */
    void onChannelsChanged();
    /** @brief 主题切换响应，更新图表视觉元素 */
    void onThemeChanged();

private:
    /** @brief 初始化UI布局(图表+工具栏) */
    void setupUI();
    /** @brief 创建顶部配置工具栏 @return 工具栏Widget指针 */
    QWidget* createToolbar();
    /** @brief 创建直方图图表区域(QChartView/QBarSeries/坐标轴) */
    void setupChart();
    /** @brief 应用当前主题颜色到图表(背景/网格/轴标签/柱体颜色) */
    void applyThemeColors();

    ChartModel* m_model;              ///< 数据模型（外部拥有）
    QChartView* m_chartView;          ///< 图表视图
    QChart* m_chart;                  ///< Qt Charts 图表对象
    QBarSeries* m_series;             ///< 直方图柱体序列
    QBarSet* m_barSet;                ///< 直方图柱体数据集
    QBarCategoryAxis* m_xAxis;        ///< X轴（分桶类别）
    QValueAxis* m_yAxis;              ///< Y轴（计数）
    QComboBox* m_channelCombo;        ///< 通道选择下拉框
    QSpinBox* m_binsSpin;             ///< 分桶数输入框 (5~200, 默认30)
    QPushButton* m_refreshBtn;        ///< 手动刷新按钮
    QCheckBox* m_autoRefreshCheck;    ///< 自动刷新复选框
    QLabel* m_statsLabel;             ///< 统计摘要标签
    bool m_autoRefresh = false;       ///< 是否自动刷新

    // ---- 统计计数器 ----
    quint64 m_totalUpdates = 0;       ///< 累计刷新更新次数
    quint64 m_totalBinChanges = 0;    ///< 累计分桶数变更次数
    quint64 m_totalBinsComputed = 0;  ///< 累计分桶计算次数
    int m_peakBinIndex = -1;          ///< 峰值所在桶索引
    int m_maxBinCount = 0;            ///< 最大桶计数值
    quint64 m_totalBinRecalculations = 0;  ///< 累计分桶重计算次数
    quint64 m_totalDistributionUpdates = 0;///< 累计分布更新次数
    quint64 m_totalAutoRanges = 0;    ///< 累计Y轴自动范围调整次数
    quint64 m_totalExports = 0;       ///< 累计导出次数
    quint64 m_totalChannelSwitches = 0; ///< 累计通道切换次数
    quint64 m_totalAutoRefreshToggles = 0; ///< 累计自动刷新开关切换次数
    quint64 m_totalThemeChanges = 0;  ///< 累计主题颜色变更次数
};

#endif // HISTOGRAMWIDGET_H
