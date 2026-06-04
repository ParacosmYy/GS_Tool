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
    /** @brief 构造波形图控件 @param parent 父控件指针 */
    explicit ChartWidget(QWidget* parent = nullptr);
    /** @brief 获取内部数据模型 @return ChartModel指针 */
    ChartModel* model() const;
    /** @brief 从帧定义自动生成通道配置 @param def 帧定义结构体 */
    void configureFromFrameDefinition(const FrameDefinition& def);
    /** @brief 设置滑动窗口大小 @param points 窗口内最大数据点数 */
    void setWindowSize(int points);
    /** @brief 清除所有通道数据 */
    void clear();
    /** @brief 获取当前通道名称列表 @return 通道名称QStringList */
    QStringList channels() const;
    /** @brief 设置Y轴固定范围(禁用自动) @param min Y轴下界 @param max Y轴上界 */
    void setYRange(double min, double max);
    /** @brief 启用/禁用自动Y轴范围 @param enabled true=自动调整 */
    void setAutoYRange(bool enabled);

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
    /** @brief 获取数据更新总次数 @return 累计更新次数 */
    quint64 totalDataUpdates() const { return m_totalDataUpdates; }
    /** @brief 获取渲染总次数 @return 累计渲染次数 */
    quint64 totalRenders() const { return m_totalRenders; }
    /** @brief 获取重绘总次数 @return 累计重绘次数 */
    quint64 totalRedraws() const { return m_totalRedraws; }
    /** @brief 获取交互操作总次数 @return 累计交互次数 */
    quint64 totalInteractions() const { return m_totalInteractions; }
    /** @brief 获取缩放事件总次数 @return 累计缩放次数 */
    quint64 totalZoomEvents() const { return m_totalZoomEvents; }
    /** @brief 获取平移事件总次数 @return 累计平移次数 */
    quint64 totalPanEvents() const { return m_totalPanEvents; }
    /** @brief 获取通道切换总次数 @return 累计通道切换次数 */
    quint64 totalChannelToggles() const { return m_totalChannelToggles; }
    /** @brief 获取截图总次数 @return 累计截图次数 */
    quint64 totalScreenshots() const { return m_totalScreenshots; }
    /** @brief 获取采样点追加总次数 @return 累计采样点数 */
    quint64 totalSamplesAppended() const { return m_totalSamplesAppended; }
    /** @brief 获取自动缩放总次数 @return 累计自动缩放次数 */
    quint64 totalAutoScales() const { return m_totalAutoScales; }
    /** @brief 获取手动缩放总次数 @return 累计手动缩放次数 */
    quint64 totalManualZooms() const { return m_totalManualZooms; }
    /** @brief 获取游标移动总次数 @return 累计游标移动次数 */
    quint64 totalCursorMoves() const { return m_cursorOverlay ? m_cursorOverlay->totalCursorMoves() : 0; }
    /** @brief 获取当前渲染帧率 @return FPS值 */
    double renderFps() const { return m_renderFps; }
    /** @brief 导出图表为PNG截图 @param filePath 输出文件路径 @return 成功返回true */
    bool exportScreenshot(const QString& filePath);
    /** @brief 重置图表控件统计计数器 */
    void resetChartWidgetStatistics();
};

#endif // CHARTWIDGET_H
