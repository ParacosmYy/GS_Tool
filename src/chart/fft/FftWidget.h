/**
 * @file FftWidget.h
 * @brief FFT频谱显示控件 -- 主题感知的频谱分析面板
 *
 * 设计要点:
 *   1. 自包含QWidget，可直接作为标签页或停靠窗口添加
 *   2. 从ChartModel读取通道时域数据，经FftEngine计算后渲染频谱
 *   3. 支持通道选择、窗函数选择、FFT大小配置、手动/自动刷新
 *   4. 主题感知: 背景色、网格线、标签色跟随ThemeManager
 *
 * 协作关系:
 *   - ChartModel: 提供通道时域数据（channelData方法）
 *   - FftEngine: 执行FFT计算，返回频谱数据
 *   - ThemeManager: 监听themeChanged信号更新视觉样式
 *   - ChartColors: 频谱线条颜色使用主题调色板
 */

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
 *
 * 独立面板控件，内含频谱图表和配置工具栏。
 * 从ChartModel读取指定通道的时域数据，计算FFT并显示频谱。
 *
 * 功能:
 *   - 通道选择: 下拉框选择要分析的通道
 *   - 窗函数选择: Rectangular/Hanning/Hamming/Blackman
 *   - FFT大小: 256/512/1024/2048/4096
 *   - 手动刷新按钮 + 自动刷新复选框
 *   - 采样率配置（用户输入或从数据推算）
 */
class FftWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造FFT频谱控件
     * @param model 数据模型指针（外部拥有，不负责销毁）
     * @param parent 父控件
     */
    explicit FftWidget(ChartModel* model, QWidget* parent = nullptr);

    /**
     * @brief 设置采样率（Hz）
     *
     * 用于计算频率轴。若未设置则默认为1000Hz。
     * @param rate 采样率（Hz）
     */
    void setSampleRate(double rate);

    /** @brief 获取当前采样率 */
    double sampleRate() const;

public slots:
    /**
     * @brief 刷新频谱显示
     *
     * 从ChartModel读取当前选中通道的数据，
     * 经FftEngine计算后更新频谱图表。
     */
    void refreshSpectrum();

private slots:
    /** @brief 通道选择变更 */
    void onChannelChanged(int index);

    /** @brief 窗函数选择变更 */
    void onWindowChanged(int index);

    /** @brief FFT大小变更 */
    void onFftSizeChanged(int value);

    /** @brief 自动刷新开关切换 */
    void onAutoRefreshToggled(bool checked);

    /** @brief ChartModel数据更新时（自动刷新模式下触发重算） */
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

    /** @brief 创建频谱图表区域 */
    void setupChart();

    /**
     * @brief 应用当前主题颜色到图表
     *
     * 更新内容:
     *   - 图表背景色 (BgPrimary)
     *   - 网格线颜色 (Border)
     *   - 坐标轴标签颜色 (TextSecondary)
     *   - 频谱线条颜色 (ChartColors调色板第一色)
     */
    void applyThemeColors();

    /** @brief 填充FFT大小下拉框 */
    void populateFftSizes();

    ChartModel* m_model;              ///< 数据模型（外部拥有）
    FftEngine* m_engine;              ///< FFT计算引擎

    // ---- 图表组件 ----
    QChartView* m_chartView;          ///< 图表视图
    QChart* m_chart;                  ///< Qt Charts 图表对象
    QLineSeries* m_spectrumSeries;    ///< 频谱曲线
    QValueAxis* m_xAxis;             ///< X轴（频率 Hz）
    QValueAxis* m_yAxis;             ///< Y轴（幅度）

    // ---- 工具栏控件 ----
    QComboBox* m_channelCombo;        ///< 通道选择下拉框
    QComboBox* m_windowCombo;         ///< 窗函数选择下拉框
    QComboBox* m_fftSizeCombo;        ///< FFT大小下拉框
    QSpinBox* m_sampleRateSpin;       ///< 采样率输入框
    QPushButton* m_refreshBtn;        ///< 手动刷新按钮
    QCheckBox* m_autoRefreshCheck;    ///< 自动刷新复选框
    QLabel* m_infoLabel;              ///< 信息标签（基频/点数等）

    // ---- 状态 ----
    double m_sampleRate = 1000.0;     ///< 采样率（Hz）
    bool m_autoRefresh = false;       ///< 是否自动刷新
    double m_fundamentalFreq = 0.0;   ///< 最近一次计算的基频
};

#endif // FFTWIDGET_H
