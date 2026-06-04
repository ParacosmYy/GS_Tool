/**
 * @file HistogramWidget.h
 * @brief 统计直方图控件 -- 通道数据值分布分析面板
 *
 * 设计要点:
 *   1. 自包含QWidget，可直接作为标签页或停靠窗口添加
 *   2. 从ChartModel读取通道数据，计算值分布直方图并渲染
 *   3. 支持通道选择、分桶数配置、手动/自动刷新
 *   4. 显示统计摘要: 均值、标准差、最小值、最大值、样本数
 *   5. 主题感知: 背景色、网格线、标签色跟随ThemeManager
 *
 * 协作关系:
 *   - ChartModel: 提供通道数据（channelData方法）
 *   - ThemeManager: 监听themeChanged信号更新视觉样式
 *   - ChartColors: 直方图柱体颜色使用主题调色板
 */

#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QVector>
#include <QPointF>
#include <QPair>

#include "chart/widget/ChartColors.h"

class QChartView;
class QChart;
class QBarSeries;
class QBarSet;
class QBarCategoryAxis;
class QValueAxis;
class QComboBox;
class QSpinBox;
class QPushButton;
class QCheckBox;
class QLabel;
class ChartModel;

/**
 * @brief 统计直方图控件
 *
 * 独立面板控件，内含直方图图表和配置工具栏。
 * 从ChartModel读取指定通道的数据，计算值分布直方图并显示统计摘要。
 */
class HistogramWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 统计摘要数据结构 */
    struct Stats {
        double mean;    ///< 均值
        double stddev;  ///< 标准差
        double min;     ///< 最小值
        double max;     ///< 最大值
        int count;      ///< 样本数
    };

    /**
     * @brief 构造直方图控件
     * @param model 数据模型指针（外部拥有，不负责销毁）
     * @param parent 父控件
     */
    explicit HistogramWidget(ChartModel* model, QWidget* parent = nullptr);

    /**
     * @brief 计算直方图分桶数据
     * @param data 输入数据点集（使用Y值）
     * @param bins 分桶数量
     * @return (分桶中心值, 计数) 对的向量
     */
    static QVector<QPair<double, int>> computeHistogram(
        const QVector<QPointF>& data, int bins);

    /**
     * @brief 计算统计摘要
     * @param data 输入数据点集（使用Y值）
     * @return 统计摘要结构
     */
    static Stats computeStats(const QVector<QPointF>& data);

public slots:
    /** @brief 刷新直方图显示 -- 从ChartModel读取数据并重新计算 */
    void refreshHistogram();

    /** @brief 导出当前直方图数据到CSV文件 @param filePath 目标文件路径 @return true=导出成功 */
    bool exportToCsv(const QString& filePath);

    // ---- 统计计数接口 ----
    /** @brief 获取累计刷新更新次数 */
    quint64 totalUpdates() const;
    /** @brief 获取累计分桶数变更次数 */
    quint64 totalBinChanges() const;
    /** @brief 获取累计分桶计算次数 */
    quint64 totalBinsComputed() const;
    /** @brief 获取峰值所在桶索引(上次计算结果) @return 桶索引，无数据返回-1 */
    int peakBinIndex() const;
    /** @brief 获取最大桶计数值(上次计算结果) @return 最大计数 */
    int maxBinCount() const;
    /** @brief 获取累计分桶重计算次数(computeHistogram调用) */
    quint64 totalBinRecalculations() const { return m_totalBinRecalculations; }
    /** @brief 获取累计分布更新次数(refreshHistogram成功执行) */
    quint64 totalDistributionUpdates() const { return m_totalDistributionUpdates; }
    /** @brief 获取累计Y轴自动范围调整次数 */
    quint64 totalAutoRanges() const { return m_totalAutoRanges; }
    /** @brief 获取累计导出次数(CSV导出调用，无论成功与否) */
    quint64 totalExports() const { return m_totalExports; }
    /** @brief 重置所有直方图统计计数器 */
    void resetHistogramStatistics();

private slots:
    void onChannelChanged(int index);           ///< 通道选择变更
    void onBinsChanged(int value);              ///< 分桶数变更
    void onAutoRefreshToggled(bool checked);    ///< 自动刷新开关切换
    void onDataUpdated(const QStringList& updatedChannels);  ///< 数据更新
    void onChannelsChanged();                   ///< 通道列表变更
    void onThemeChanged();                      ///< 主题切换响应

private:
    void setupUI();         ///< 初始化UI布局
    QWidget* createToolbar();  ///< 创建顶部配置工具栏
    void setupChart();      ///< 创建直方图图表区域
    void applyThemeColors();   ///< 应用当前主题颜色到图表

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
    int m_peakBinIndex = -1;          ///< 峰值所在桶索引(上次计算结果)
    int m_maxBinCount = 0;            ///< 最大桶计数值(上次计算结果)
    quint64 m_totalBinRecalculations = 0;  ///< 累计分桶重计算次数(computeHistogram调用)
    quint64 m_totalDistributionUpdates = 0;///< 累计分布更新次数(refreshHistogram成功执行)
    quint64 m_totalAutoRanges = 0;    ///< 累计Y轴自动范围调整次数
    quint64 m_totalExports = 0;       ///< 累计导出次数(CSV导出调用)
};

#endif // HISTOGRAMWIDGET_H
