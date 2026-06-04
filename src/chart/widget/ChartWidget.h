/** @file ChartWidget.h @brief 实时波形图控件 -- 基于Qt Charts的滑动窗口波形显示。多通道QLineSeries / ChartModel / ThemeManager / ChartColors */
#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QMap>
#include <QColor>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "chart/widget/ChartColors.h"
#include "chart/model/ChannelConfig.h"
#include "chart/model/ChartModel.h"
#include "chart/overlay/CursorOverlay.h"
#include "chart/zoom/ZoomController.h"
#include "chart/scale/YAxisManager.h"

struct FrameDefinition;

/** @brief 实时波形图控件 -- 渲染层(工具栏+QChart渲染+主题响应)。设计模式: 观察者+单一职责 */
class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr); ///< 构造波形图控件
    ChartModel* model() const;                       ///< 获取内部ChartModel
    void configureFromFrameDefinition(const FrameDefinition& def); ///< 从帧定义自动生成通道配置
    void setWindowSize(int points);                  ///< 设置滑动窗口大小
    void clear();                                    ///< 清除所有通道数据
    QStringList channels() const;                    ///< 获取当前通道名称列表
    void setYRange(double min, double max);          ///< 设置Y轴固定范围(禁用自动)
    void setAutoYRange(bool enabled);                ///< 启用/禁用自动Y轴范围

public slots:
    /** @brief 帧解析回调(兼容旧接口，委托给ChartModel) */
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

private slots:
    void updateChart(const QStringList& updatedChannels); ///< ChartModel::dataUpdated触发渲染刷新
    void onChannelsChanged();              ///< 通道重建
    void onDataCleared();                  ///< 数据清除
    void onPauseToggled(bool paused);      ///< 暂停/继续切换
    void onClearClicked();                 ///< 清除按钮
    void onThemeChanged();                 ///< 主题切换(背景/网格/轴标签/图例/数据线颜色)

private:
    void setupUI();               ///< 初始化UI布局和控件
    QWidget* createToolbar();     ///< 创建顶部工具栏
    void createSeries(const QString& name, const QColor& color);  ///< 创建通道QLineSeries
    void removeSeries(const QString& name);                        ///< 移除通道QLineSeries
    void applyThemeColors();      ///< 应用当前主题颜色到图表所有视觉元素
    void resizeEvent(QResizeEvent* event) override; ///< 同步游标叠加层尺寸
    // ---- 图表组件 ----
    QChartView* m_chartView;          ///< 图表视图控件
    QChart* m_chart;                  ///< Qt Charts 图表对象
    QValueAxis* m_xAxis;             ///< X轴(采样序号)
    ChartModel* m_model;              ///< 数据模型(通道配置/滑动窗口/降采样)
    ChannelConfigSet m_configSet;     ///< 通道配置集合
    YAxisManager* m_yAxisManager;     ///< 多通道独立Y轴管理器
    QMap<QString, QLineSeries*> m_seriesMap; ///< 通道名->QLineSeries映射
    bool m_autoYRange = true;         ///< 是否自动调整Y轴范围
    // ---- 控制栏控件 ----
    QPushButton* m_pauseBtn;          ///< 暂停/继续按钮
    QPushButton* m_clearBtn;          ///< 清除数据按钮
    QComboBox* m_windowSizeCombo;     ///< 窗口大小选择框
    QLabel* m_statusLabel;            ///< 状态信息标签
    bool m_paused = false;            ///< 是否暂停数据更新
    // ---- 波形交互组件 ----
    CursorOverlay* m_cursorOverlay;   ///< 游标测量叠加层
    ZoomController* m_zoomController; ///< 缩放/平移控制器
    // ---- 统计计数器 ----
    quint64 m_totalDataUpdates = 0, m_totalRenders = 0, m_totalRedraws = 0;
    quint64 m_totalInteractions = 0, m_totalZoomEvents = 0, m_totalPanEvents = 0;
    quint64 m_totalChannelToggles = 0, m_totalScreenshots = 0;
    quint64 m_totalSamplesAppended = 0, m_totalAutoScales = 0, m_totalManualZooms = 0;
    // ---- FPS追踪 ----
    qint64 m_lastRenderTimeMs = 0;    ///< 上次渲染时间戳(ms)
    double m_renderFps = 0.0;         ///< 当前渲染FPS
public:
    quint64 totalDataUpdates() const { return m_totalDataUpdates; }
    quint64 totalRenders() const { return m_totalRenders; }
    quint64 totalRedraws() const { return m_totalRedraws; }
    quint64 totalInteractions() const { return m_totalInteractions; }
    quint64 totalZoomEvents() const { return m_totalZoomEvents; }
    quint64 totalPanEvents() const { return m_totalPanEvents; }
    quint64 totalChannelToggles() const { return m_totalChannelToggles; }
    quint64 totalScreenshots() const { return m_totalScreenshots; }
    quint64 totalSamplesAppended() const { return m_totalSamplesAppended; }
    quint64 totalAutoScales() const { return m_totalAutoScales; }
    quint64 totalManualZooms() const { return m_totalManualZooms; }
    quint64 totalCursorMoves() const { return m_cursorOverlay ? m_cursorOverlay->totalCursorMoves() : 0; }
    double renderFps() const { return m_renderFps; }
    bool exportScreenshot(const QString& filePath); ///< 导出PNG截图
    void resetChartWidgetStatistics();              ///< 重置统计计数器
};

#endif // CHARTWIDGET_H
