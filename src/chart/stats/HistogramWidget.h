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
    struct Stats { double mean; double stddev; double min; double max; int count; };

    explicit HistogramWidget(ChartModel* model, QWidget* parent = nullptr);
    static QVector<QPair<double, int>> computeHistogram(const QVector<QPointF>& data, int bins);
    static Stats computeStats(const QVector<QPointF>& data);

public slots:
    void refreshHistogram();                              ///< 刷新直方图(从ChartModel读取数据重算)
    bool exportToCsv(const QString& filePath);            ///< 导出当前直方图数据到CSV

    // ---- 统计计数接口 ----
    quint64 totalUpdates() const;
    quint64 totalBinChanges() const;
    quint64 totalBinsComputed() const;
    int peakBinIndex() const;
    int maxBinCount() const;
    quint64 totalBinRecalculations() const;
    quint64 totalDistributionUpdates() const;
    quint64 totalAutoRanges() const;
    quint64 totalExports() const;
    quint64 totalChannelSwitches() const;
    quint64 totalAutoRefreshToggles() const;
    quint64 totalThemeChanges() const;
    void resetHistogramStatistics();

private slots:
    void onChannelChanged(int index);
    void onBinsChanged(int value);
    void onAutoRefreshToggled(bool checked);
    void onDataUpdated(const QStringList& updatedChannels);
    void onChannelsChanged();
    void onThemeChanged();

private:
    void setupUI();
    QWidget* createToolbar();
    void setupChart();
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
