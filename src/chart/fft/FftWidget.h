/** @file FftWidget.h @brief FFT频谱显示控件 -- 主题感知的频谱分析面板。从ChartModel读时域数据经FftEngine计算后渲染频谱，支持通道选择/窗函数/FFT大小/手动自动刷新 */
#ifndef FFTWIDGET_H
#define FFTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QVector>
#include <QPointF>
#include <QMap>
#include <QString>
#include "chart/fft/FftEngine.h"
#include "chart/widget/ChartColors.h"

class QChartView;
class QChart;
class QLineSeries;
class QValueAxis;
class QComboBox;
class QSpinBox;
class QPushButton;
class QCheckBox;
class QLabel;
class ChartModel;

/**
 * @brief FFT频谱显示控件
 * 独立面板: 通道选择/窗函数(Rectangular/Hanning/Hamming/Blackman)/FFT大小(256~4096)/手动+自动刷新/采样率配置
 * 协作: ChartModel(数据) / FftEngine(计算) / ThemeManager(主题) / ChartColors(调色板)
 */
class FftWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造FFT频谱控件 @param model 数据模型(外部拥有) @param parent 父控件 */
    explicit FftWidget(ChartModel* model, QWidget* parent = nullptr);
    /** @brief 设置采样率(Hz)，默认1000Hz @param rate 采样率 */
    void setSampleRate(double rate);
    /** @brief 获取当前采样率 @return 采样率(Hz) */
    double sampleRate() const;
    // ---- 统计计数接口 ----
    /** @brief 获取累计FFT变换次数 @return FFT变换计数 */
    quint64 totalTransforms() const;
    /** @brief 获取峰值搜索次数 @return 峰值搜索计数 */
    quint64 totalPeakSearches() const;
    /** @brief 获取窗函数变更次数 @return 窗函数变更计数 */
    quint64 totalWindowChanges() const;
    /** @brief 获取FFT大小变更次数 @return FFT大小变更计数 */
    quint64 totalSizeChanges() const;
    /** @brief 获取累计自动刷新触发次数 @return 自动刷新计数 */
    quint64 totalAutoRefreshes() const { return m_totalAutoRefreshes; }
    /** @brief 获取累计通道切换次数 @return 通道切换计数 */
    quint64 totalChannelSwitches() const { return m_totalChannelSwitches; }
    /** @brief 获取累计渲染错误次数(计算失败/空结果) @return 错误计数 */
    quint64 totalRenderErrors() const { return m_totalRenderErrors; }
    /** @brief 重置统计计数器 */
    void resetFftWidgetStatistics();

public slots:
    /** @brief 刷新频谱(从ChartModel读数据经FftEngine计算更新图表) */
    void refreshSpectrum();

private slots:
    /** @brief 通道选择变更处理 @param index 下拉框选中索引 */
    void onChannelChanged(int index);
    /** @brief 窗函数选择变更处理 @param index 下拉框选中索引 */
    void onWindowChanged(int index);
    /** @brief FFT大小变更处理 @param value 新的FFT大小 */
    void onFftSizeChanged(int value);
    /** @brief 自动刷新开关切换 @param checked 是否启用 */
    void onAutoRefreshToggled(bool checked);
    /** @brief 数据更新(自动刷新模式触发重算) @param updatedChannels 已更新的通道列表 */
    void onDataUpdated(const QStringList& updatedChannels);
    /** @brief 通道列表变更更新下拉框 */
    void onChannelsChanged();
    /** @brief 主题切换更新图表视觉元素 */
    void onThemeChanged();

private:
    /** @brief 初始化UI布局(图表+工具栏) */
    void setupUI();
    /** @brief 创建顶部配置工具栏 @return 工具栏Widget指针 */
    QWidget* createToolbar();
    /** @brief 创建频谱图表区域(QChartView/QLineSeries/坐标轴) */
    void setupChart();
    /** @brief 应用当前主题颜色到图表(背景/网格/轴标签/频谱线) */
    void applyThemeColors();
    /** @brief 填充FFT大小下拉框(256/512/1024/2048/4096) */
    void populateFftSizes();
    // ---- 数据与引擎 ----
    ChartModel* m_model;              ///< 数据模型(外部拥有)
    FftEngine* m_engine;              ///< FFT计算引擎
    // ---- 图表组件 ----
    QChartView* m_chartView;          ///< 图表视图
    QChart* m_chart;                  ///< Qt Charts图表对象
    QLineSeries* m_spectrumSeries;    ///< 频谱曲线
    QValueAxis* m_xAxis, * m_yAxis;  ///< X轴(频率Hz)/Y轴(幅度)
    // ---- 工具栏控件 ----
    QComboBox* m_channelCombo;        ///< 通道选择
    QComboBox* m_windowCombo;         ///< 窗函数选择
    QComboBox* m_fftSizeCombo;        ///< FFT大小
    QSpinBox* m_sampleRateSpin;       ///< 采样率输入
    QPushButton* m_refreshBtn;        ///< 手动刷新按钮
    QCheckBox* m_autoRefreshCheck;    ///< 自动刷新
    QLabel* m_infoLabel;              ///< 信息标签(基频/点数)
    // ---- 状态 ----
    double m_sampleRate = 1000.0;     ///< 采样率(Hz)
    bool m_autoRefresh = false;       ///< 是否自动刷新
    double m_fundamentalFreq = 0.0;   ///< 最近基频
    // ---- 统计计数器 ----
    quint64 m_totalTransforms = 0, m_totalPeakSearches = 0;
    quint64 m_totalWindowChanges = 0, m_totalSizeChanges = 0;
    quint64 m_totalAutoRefreshes = 0;      ///< 累计自动刷新触发次数
    quint64 m_totalChannelSwitches = 0;    ///< 累计通道切换次数
    quint64 m_totalRenderErrors = 0;       ///< 累计渲染错误次数(计算失败/空结果)
};

#endif // FFTWIDGET_H
