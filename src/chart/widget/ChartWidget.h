/**
 * @file ChartWidget.h
 * @brief 实时波形图控件 -- 基于Qt Charts的滑动窗口波形显示
 *
 * 设计要点:
 *   - 支持多通道同时显示，每个通道一条 QLineSeries 曲线
 *   - 数据层由 ChartModel 管理（滑动窗口 + 降采样），本类只负责渲染
 *   - 主题切换时通过 ThemeManager 语义色板更新图表背景、网格、轴标签、图例
 *   - 数据线颜色通过 ChartColors 主题感知调色板获取
 *
 * 协作关系:
 *   - ChartModel: 数据模型，通过 dataUpdated/channelsChanged/dataCleared 信号驱动渲染
 *   - ThemeManager: 监听 themeChanged 信号，更新图表视觉元素
 *   - ChartColors: 提供主题感知的通道颜色调色板
 *   - ChannelConfigSet: 管理通道配置（名称、颜色、数据源映射）
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

/**
 * @brief 实时波形图控件 -- 渲染层
 *
 * 职责:
 *   1. 创建和布局工具栏（暂停/清除/窗口大小/状态标签）
 *   2. 管理 QChart / QChartView / QValueAxis 的创建和样式
 *   3. 根据 ChartModel 的信号刷新曲线数据
 *   4. 响应 ThemeManager::themeChanged 更新图表视觉样式
 *   5. 响应主题切换更新数据线颜色
 *
 * 设计模式:
 *   - 观察者模式: 监听 ChartModel 和 ThemeManager 的信号
 *   - 单一职责: 只负责渲染，数据逻辑在 ChartModel 中
 */
class ChartWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造波形图控件
     * @param parent 父窗口
     */
    explicit ChartWidget(QWidget* parent = nullptr);

    /** @brief 获取内部ChartModel，用于外部连接信号 */
    ChartModel* model() const;

    /**
     * @brief 从帧定义自动生成通道配置并应用
     * @param def 帧定义结构
     */
    void configureFromFrameDefinition(const FrameDefinition& def);

    /**
     * @brief 设置滑动窗口大小（显示的最大数据点数）
     * @param points 窗口大小
     */
    void setWindowSize(int points);

    /** @brief 清除所有通道数据 */
    void clear();

    /** @brief 获取当前通道名称列表 */
    QStringList channels() const;

    /**
     * @brief 手动设置Y轴范围（关闭自动范围）
     * @param min 最小值
     * @param max 最大值
     */
    void setYRange(double min, double max);

    /**
     * @brief 启用/禁用自动Y轴范围
     * @param enabled true 启用自动范围
     */
    void setAutoYRange(bool enabled);

public slots:
    /**
     * @brief 从帧解析结果中提取数值字段并添加到对应通道
     * 兼容旧接口，委托给 ChartModel
     * @param fields 帧字段键值对
     * @param rawFrame 原始帧数据（未使用）
     */
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
};

#endif // CHARTWIDGET_H
