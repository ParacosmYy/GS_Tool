/**
 * @file ChartWidget.h
 * @brief 实时波形图控件 -- 基于Qt Charts的滑动窗口波形显示
 * 设计要点: 多通道QLineSeries / ChartModel数据层 / ThemeManager主题色板 / ChartColors调色板
 * 协作: ChartModel(数据) / ThemeManager(主题) / ChartColors(调色板) / ChannelConfigSet(通道配置)
 */

#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QMap>
#include <QColor>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include "chart/widget/ChartColors.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

#include "chart/model/ChannelConfig.h"
#include "chart/model/ChartModel.h"
#include "chart/overlay/CursorOverlay.h"
#include "chart/zoom/ZoomController.h"
#include "chart/scale/YAxisManager.h"

struct FrameDefinition;

/** @brief 实时波形图控件 -- 渲染层(工具栏+QChart渲染+主题响应) 设计模式: 观察者+单一职责 */
class ChartWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造波形图控件 @param parent 父窗口 */
    explicit ChartWidget(QWidget* parent = nullptr);

    /** @brief 获取内部ChartModel，用于外部连接信号 */
    ChartModel* model() const;

    /** @brief 从帧定义自动生成通道配置并应用 @param def 帧定义结构 */
    void configureFromFrameDefinition(const FrameDefinition& def);

    /** @brief 设置滑动窗口大小 @param points 窗口大小 */
    void setWindowSize(int points);

    /** @brief 清除所有通道数据 */
    void clear();

    /** @brief 获取当前通道名称列表 */
    QStringList channels() const;

    /** @brief 设置Y轴固定范围(禁用自动Y轴) @param min 最小值 @param max 最大值 */
    void setYRange(double min, double max);

    /** @brief 启用/禁用自动Y轴范围 @param enabled true=启用 */
    void setAutoYRange(bool enabled);

public slots:
    /** @brief 帧解析回调(兼容旧接口，委托给ChartModel) @param fields 字段映射 @param rawFrame 原始帧 */
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

private slots:
    /** @brief ChartModel::dataUpdated 触发的渲染刷新 */
    void updateChart(const QStringList& updatedChannels);

    /** @brief ChartModel::channelsChanged 触发的通道重建 */
    void onChannelsChanged();

    /** @brief ChartModel::dataCleared 触发的清除 */
    void onDataCleared();

    /** @brief 暂停/继续按钮切换 */
    void onPauseToggled(bool paused);

    /** @brief 清除按钮点击 */
    void onClearClicked();

    /**
     * @brief 主题切换响应 -- 更新图表所有视觉元素
     *
     * 更新内容:
     *   - 图表背景色 (BgPrimary)
     *   - 网格线颜色 (Border)
     *   - 坐标轴标签颜色 (TextSecondary)
     *   - 图例文字颜色 (TextSecondary)
     *   - 数据线颜色 (ChartColors 主题调色板)
     */
    void onThemeChanged();

private:
    /** @brief 初始化UI布局和控件 */
    void setupUI();

    /**
     * @brief 创建顶部工具栏(暂停/清除/游标/窗口大小/状态标签)
     * @return 工具栏Widget指针
     */
    QWidget* createToolbar();

    /**
     * @brief 为指定通道创建 QLineSeries 并添加到图表
     * @param name 通道名称
     * @param color 通道颜色（无效时使用调色板默认色）
     */
    void createSeries(const QString& name, const QColor& color);

    /**
     * @brief 移除指定通道的 QLineSeries
     * @param name 通道名称
     */
    void removeSeries(const QString& name);

    /**
     * @brief 应用当前主题颜色到图表所有视觉元素
     *
     * 从 ThemeManager 获取语义色值并应用到:
     *   - QChart 背景
     *   - 坐标轴网格线和标签
     *   - 图例文字
     * 同时从 ChartColors 获取主题调色板并更新所有已有数据线颜色
     */
    void applyThemeColors();

    /** @brief 窗口大小变化时同步游标叠加层尺寸 */
    void resizeEvent(QResizeEvent* event) override;

    QChartView* m_chartView;          ///< 图表视图控件
    QChart* m_chart;                  ///< Qt Charts 图表对象
    QValueAxis* m_xAxis;             ///< X轴（采样序号）

    ChartModel* m_model;              ///< 数据模型（管理通道配置、滑动窗口、降采样）
    ChannelConfigSet m_configSet;     ///< 通道配置集合
    YAxisManager* m_yAxisManager;     ///< 多通道独立Y轴管理器

    /** @brief 通道名 -> QLineSeries 映射（仅用于渲染层） */
    QMap<QString, QLineSeries*> m_seriesMap;

    bool m_autoYRange = true;         ///< 是否自动调整Y轴范围

    // ---- 控制栏控件 ----
    QPushButton* m_pauseBtn;          ///< 暂停/继续按钮
    QPushButton* m_clearBtn;          ///< 清除数据按钮
    QComboBox* m_windowSizeCombo;     ///< 窗口大小选择框
    QLabel* m_statusLabel;            ///< 状态信息标签（通道数/帧数）

    bool m_paused = false;            ///< 是否暂停数据更新

    // ---- 波形交互组件 ----
    CursorOverlay* m_cursorOverlay;   ///< 游标测量叠加层
    ZoomController* m_zoomController; ///< 缩放/平移控制器

    // ---- 统计计数器 ----
    quint64 m_totalDataUpdates = 0;    ///< 总数据更新次数
    quint64 m_totalRenders = 0;        ///< 总渲染次数
    quint64 m_totalInteractions = 0;   ///< 总交互次数(暂停/清除等)

public:
    /** @brief 获取总数据更新次数 @return 数据更新计数 */
    quint64 totalDataUpdates() const { return m_totalDataUpdates; }
    /** @brief 获取总渲染次数 @return 渲染计数 */
    quint64 totalRenders() const { return m_totalRenders; }
    /** @brief 获取总交互次数 @return 交互计数 */
    quint64 totalInteractions() const { return m_totalInteractions; }
    /** @brief 重置波形图统计计数器 */
    void resetChartWidgetStatistics();
};

#endif // CHARTWIDGET_H
