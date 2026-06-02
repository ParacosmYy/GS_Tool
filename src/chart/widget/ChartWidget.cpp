/**
 * @file ChartWidget.cpp
 * @brief 实时波形图控件实现
 *
 * 实现要点:
 *   1. 构造时连接 ChartModel 信号 + ThemeManager::themeChanged 信号
 *   2. 主题切换时通过 ThemeManager::color() 更新图表背景/网格/轴标签/图例
 *   3. 主题切换时通过 ChartColors::colorsForTheme() 更新数据线颜色
 *   4. 数据线颜色随主题变化，确保在暗色/亮色背景下均清晰可读
 */

#include "chart/widget/ChartWidget.h"
#include "protocol/parser/FrameDefinition.h"
#include "core/theme/ThemeManager.h"
#include "core/widgets/AnimatedButton.h"

#include <QtCharts>
#include <QResizeEvent>
#include <algorithm>

// ============================================================================
// 构造函数
// ============================================================================

/** @brief 构造波形图控件(创建ChartModel+初始化UI+连接信号) @param parent 父控件 */
ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
    , m_model(new ChartModel(this))
{
    setupUI();

    // ---- 连接 ChartModel 信号到渲染槽 ----
    connect(m_model, &ChartModel::dataUpdated,
            this, &ChartWidget::updateChart);
    connect(m_model, &ChartModel::channelsChanged,
            this, &ChartWidget::onChannelsChanged);
    connect(m_model, &ChartModel::dataCleared,
            this, &ChartWidget::onDataCleared);

    // ---- 连接 ThemeManager 主题切换信号 ----
    // 主题切换时更新图表背景、网格、轴标签、图例和数据线颜色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &ChartWidget::onThemeChanged);

    // 初始应用当前主题颜色
    applyThemeColors();
}

// ============================================================================
// UI 初始化
// ============================================================================

/** @brief 初始化波形图UI(QChartView+工具栏+游标叠加层+缩放控制器) */
void ChartWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ---- 顶部控制栏 ----
    layout->addWidget(createToolbar());

    // ---- 图表区域 ----
    m_chart = new QChart;
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setMargins(QMargins(4, 4, 4, 4));

    m_xAxis = new QValueAxis;
    m_xAxis->setTitleText(tr("采样数"));
    m_xAxis->setLabelFormat("%d");
    m_chart->addAxis(m_xAxis, Qt::AlignBottom);

    // 多通道独立Y轴管理器（替代单一m_yAxis）
    m_yAxisManager = new YAxisManager(m_chart, this);

    // 设置初始轴范围，使图表在没有数据时也能渲染背景
    m_xAxis->setRange(0, 10);

    m_chartView = new QChartView(m_chart);
    m_chartView->setObjectName("chartView");  // QSS 选择器需要
    m_chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(m_chartView, 1);

    // ---- 游标测量叠加层 ----
    m_cursorOverlay = new CursorOverlay(m_chartView, m_model, m_chartView);
    m_cursorOverlay->setObjectName("cursorOverlay");
    m_cursorOverlay->setVisible(false);  // 默认隐藏，点游标按钮才开启

    // ---- 缩放/平移控制器 ----
    m_zoomController = new ZoomController(m_chartView, this);
    m_chartView->installEventFilter(m_zoomController);
    // CursorOverlay的eventFilter后安装 → 调用顺序: CursorOverlay先 → ZoomController后
    m_chartView->installEventFilter(m_cursorOverlay);
    // 关联缩放控制器到叠加层(用于绘制框选矩形)
    m_cursorOverlay->setZoomController(m_zoomController);
    // 缩放/平移变化时刷新游标叠加层(坐标映射变了)
    connect(m_zoomController, &ZoomController::viewChanged,
            m_cursorOverlay, qOverload<>(&QWidget::update));
}

/**
 * @brief 创建顶部工具栏
 * @return 工具栏Widget(包含暂停/清除/游标/窗口大小/状态标签)
 */
QWidget* ChartWidget::createToolbar()
{
    auto* toolbar = new QWidget;
    toolbar->setObjectName("chartToolbar");
    auto* toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(8, 4, 8, 4);

    m_pauseBtn = new AnimatedButton(tr("暂停"));
    m_pauseBtn->setObjectName("chartPauseBtn");
    m_pauseBtn->setCheckable(true);
    m_pauseBtn->setMinimumWidth(60);

    m_clearBtn = new AnimatedButton(tr("清除"));
    m_clearBtn->setObjectName("chartClearBtn");
    m_clearBtn->setMinimumWidth(50);

    // 游标开关按钮
    auto* cursorBtn = new AnimatedButton(tr("游标"));
    cursorBtn->setObjectName("chartCursorBtn");
    cursorBtn->setCheckable(true);
    cursorBtn->setToolTip(tr("开启双游标测量模式"));
    cursorBtn->setMinimumWidth(50);
    connect(cursorBtn, &QPushButton::toggled, this, [this](bool on) {
        m_cursorOverlay->setVisible(on);
        if (!on) m_cursorOverlay->clearCursors();
    });

    auto* windowLabel = new QLabel(tr("窗口:"));
    windowLabel->setObjectName("chartWindowLabel");
    m_windowSizeCombo = new QComboBox;
    m_windowSizeCombo->setObjectName("chartWindowCombo");
    m_windowSizeCombo->addItems({"100", "200", "500", "1000", "2000"});
    m_windowSizeCombo->setCurrentIndex(1);
    m_windowSizeCombo->setMinimumWidth(75);

    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("chartStatusLabel");
    m_statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    toolLayout->addWidget(m_pauseBtn);
    toolLayout->addWidget(m_clearBtn);
    toolLayout->addWidget(cursorBtn);
    toolLayout->addWidget(windowLabel);
    toolLayout->addWidget(m_windowSizeCombo);
    toolLayout->addStretch();
    toolLayout->addWidget(m_statusLabel);

    // 工具栏信号连接
    connect(m_pauseBtn, &QPushButton::toggled, this, &ChartWidget::onPauseToggled);
    connect(m_clearBtn, &QPushButton::clicked, this, &ChartWidget::onClearClicked);
    connect(m_windowSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                static const int sizes[] = {100, 200, 500, 1000, 2000};
                if (idx >= 0 && idx < 5) {
                    setWindowSize(sizes[idx]);
                }
            });

    m_statusLabel->setText(tr("通道: 0"));
    return toolbar;
}

// ============================================================================
// 公开接口
// ============================================================================

/** @brief 返回波形数据模型指针 @return ChartModel指针 */
ChartModel* ChartWidget::model() const
{
    return m_model;
}

/** @brief 从帧定义配置波形图(自动创建通道映射) @param def 帧定义 */
void ChartWidget::configureFromFrameDefinition(const FrameDefinition& def)
{
    // 从帧定义的字段列表自动生成通道配置
    m_configSet = ChannelConfigSet::generateDefaults(def.fields);

    // 应用到ChartModel（会触发 channelsChanged 信号 -> 重建渲染层）
    m_model->setChannelConfigSet(m_configSet);
}

/** @brief 设置滑动窗口大小 @param points 窗口点数 */
void ChartWidget::setWindowSize(int points)
{
    m_model->setWindowSize(points);
}

/** @brief 清除波形数据 */
void ChartWidget::clear()
{
    m_model->clear();
}

/** @brief 返回当前通道名称列表 @return 通道名列表 */
QStringList ChartWidget::channels() const
{
    return m_model->channelNames();
}

/** @brief 设置Y轴固定范围(禁用自动Y轴) @param min 最小值 @param max 最大值 */
void ChartWidget::setYRange(double min, double max)
{
    m_autoYRange = false;
    // 设置所有已有Y轴的范围
    for (const QString& ch : m_seriesMap.keys()) {
        m_yAxisManager->updateRange(ch, min, max);
    }
}

/** @brief 设置是否启用Y轴自动范围 @param enabled true=自动 */
void ChartWidget::setAutoYRange(bool enabled)
{
    m_autoYRange = enabled;
}

// ============================================================================
// 槽函数 -- 帧数据接收（兼容旧接口，委托给ChartModel）
// ============================================================================

/** @brief 帧解析回调：转发到ChartModel @param fields 字段映射 @param rawFrame 原始帧 */
void ChartWidget::onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame)
{
    if (m_paused) return;
    m_model->onFrameParsed(fields, rawFrame);
}

// ============================================================================
// 槽函数 -- ChartModel 信号驱动的渲染更新
// ============================================================================

/** @brief 图表数据更新回调：刷新可见通道的series数据和坐标轴范围 @param updatedChannels 更新的通道名称列表 */
void ChartWidget::updateChart(const QStringList& updatedChannels)
{
    if (m_paused) return;

    // 刷新每个更新通道的series数据
    for (const QString& name : updatedChannels) {
        auto it = m_seriesMap.find(name);
        if (it == m_seriesMap.end()) continue;

        QLineSeries* series = it.value();
        QVector<QPointF> data = m_model->channelData(name);
        series->replace(data);
    }

    // 更新X轴范围
    QPair<double, double> xRange = m_model->xRange();
    m_xAxis->setRange(xRange.first, xRange.second);

    // 自动Y轴范围: 按通道独立更新
    if (m_autoYRange) {
        for (const QString& name : updatedChannels) {
            if (!m_seriesMap.contains(name)) continue;
            QPair<double, double> yRange = m_model->channelYRange(name);
            if (yRange.first != 0.0 || yRange.second != 0.0) {
                double margin = (yRange.second - yRange.first) * 0.1;
                if (margin < 0.001) margin = 1.0;
                m_yAxisManager->updateRange(name,
                    yRange.first - margin, yRange.second + margin);
            }
        }
    }

    // 更新状态标签
    m_statusLabel->setText(tr("通道: %1 | 帧: %2")
        .arg(m_seriesMap.size())
        .arg(m_model->currentFrameIndex()));
}

/** @brief 通道配置变化回调：同步series(新增/删除通道对应的QLineSeries) */
void ChartWidget::onChannelsChanged()
{
    // 清除旧的series
    for (auto* series : m_seriesMap) {
        m_chart->removeSeries(series);
        series->deleteLater();
    }
    m_seriesMap.clear();

    // 清除所有旧Y轴（全量重建）
    m_yAxisManager->clearAll();

    // 获取当前主题对应的调色板
    bool isDark = ThemeManager::instance().isSystemDarkMode()
        || ThemeManager::instance().currentTheme().contains("dark");
    QVector<QColor> palette = ChartColors::colorsForTheme(isDark);
    if (palette.isEmpty()) {
        palette = {Qt::cyan};  // 降级回退色，防止除零崩溃
    }

    // ---- 降级路径: m_configSet 尚未配置时，直接从 model 获取通道名 ----
    // 场景: 协议桥数据先于帧编辑器配置到达，此时 channelsChanged 信号已触发
    //       但 configureFromFrameDefinition() 还未被调用，m_configSet 为空
    if (m_configSet.channels().isEmpty() && m_model) {
        const QStringList names = m_model->channelNames();
        // 自动分配左右侧
        QStringList emptyUnits;
        for (int i = 0; i < names.size(); ++i) emptyUnits.append(QString());
        QVector<YAxisSide> sides = YAxisManager::autoAssignSides(names, emptyUnits);

        for (int i = 0; i < names.size(); ++i) {
            QColor chColor = palette[i % palette.size()];
            m_yAxisManager->createAxis(names[i], chColor, sides[i]);
            createSeries(names[i], chColor);
        }
        m_statusLabel->setText(tr("通道: %1").arg(m_seriesMap.size()));
        return;
    }

    // ---- 主路径: 根据完整的通道配置创建series和Y轴 ----
    const QVector<ChannelConfig>& channels = m_configSet.channels();
    QStringList names, units;
    for (const ChannelConfig& cfg : channels) {
        if (cfg.enabled) {
            names.append(cfg.displayName);
            units.append(cfg.unit);
        }
    }

    // 自动分配左右侧
    QVector<YAxisSide> sides = YAxisManager::autoAssignSides(names, units);

    int idx = 0;
    for (const ChannelConfig& cfg : channels) {
        if (!cfg.enabled) continue;
        // 如果通道配置有自定义颜色则使用，否则从主题调色板获取
        QColor chColor = cfg.color.isValid() ? cfg.color :
            palette[idx % palette.size()];

        // 创建独立Y轴
        m_yAxisManager->createAxis(cfg.displayName, chColor,
            sides[idx], cfg.unit);

        createSeries(cfg.displayName, chColor);
        ++idx;
    }

    m_statusLabel->setText(tr("通道: %1").arg(m_seriesMap.size()));
}

/** @brief 数据清空回调：清除所有series数据点 */
void ChartWidget::onDataCleared()
{
    // 清除所有series的数据点
    for (auto* series : m_seriesMap) {
        series->clear();
    }
    m_xAxis->setRange(0, 10);
    // 重置所有Y轴到默认范围
    for (const QString& ch : m_seriesMap.keys()) {
        m_yAxisManager->updateRange(ch, 0, 100);
    }
}

// ============================================================================
// 槽函数 -- 控制栏按钮
// ============================================================================

/** @brief 暂停/继续按钮切换回调 @param paused true=暂停 */
void ChartWidget::onPauseToggled(bool paused)
{
    m_paused = paused;
    m_pauseBtn->setText(paused ? tr("继续") : tr("暂停"));
}

/** @brief 清除按钮回调：清空波形数据和series */
void ChartWidget::onClearClicked()
{
    clear();
}

// ============================================================================
// 主题切换 -- 响应 ThemeManager::themeChanged 信号
// ============================================================================

/** @brief 主题切换回调：重绘所有图表视觉元素 */
void ChartWidget::onThemeChanged()
{
    applyThemeColors();
}

/** @brief 应用当前主题颜色到图表背景/坐标轴/series */
void ChartWidget::applyThemeColors()
{
    auto& theme = ThemeManager::instance();

    // 判断当前是否为暗色主题（与 onChannelsChanged 一致：同时检查系统暗色模式和主题名称）
    bool isDark = theme.isSystemDarkMode() || theme.currentTheme().contains("dark");

    // ---- 1. 图表背景色 ----
    QColor bgColor = theme.color(ThemeManager::SemanticColor::BgPrimary);
    m_chart->setBackgroundBrush(QBrush(bgColor));
    m_chart->setPlotAreaBackgroundBrush(QBrush(bgColor));
    m_chart->setPlotAreaBackgroundVisible(true);

    // ---- 2. 网格线和标签颜色 ----
    QColor gridColor = theme.color(ThemeManager::SemanticColor::Border);
    QColor labelColor = theme.color(ThemeManager::SemanticColor::TextSecondary);

    // X轴网格线和标签颜色
    m_xAxis->setGridLineColor(gridColor);
    m_xAxis->setLinePen(QPen(gridColor, 1));
    m_xAxis->setLabelsBrush(QBrush(labelColor));
    m_xAxis->setTitleBrush(labelColor);

    // Y轴: 委托给 YAxisManager 更新所有Y轴颜色
    m_yAxisManager->applyThemeColors(gridColor, labelColor);

    // ---- 3. 图例文字颜色 ----
    if (m_chart->legend()) {
        m_chart->legend()->setLabelColor(labelColor);
    }

    // ---- 4. 数据线颜色 ----
    // 从当前主题对应的调色板中按通道索引重新分配颜色
    QVector<QColor> palette = ChartColors::colorsForTheme(isDark);
    if (palette.isEmpty()) return;  // 防御性检查: 空调色板无法分配颜色
    int index = 0;
    for (auto it = m_seriesMap.begin(); it != m_seriesMap.end(); ++it, ++index) {
        QColor lineColor = palette[index % palette.size()];
        it.value()->setColor(lineColor);
    }
}

// ============================================================================
// 事件处理
// ============================================================================

/** @brief 窗口大小变化时同步游标叠加层尺寸 @param event 调整大小事件 */
void ChartWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_cursorOverlay && m_chartView) {
        m_cursorOverlay->setGeometry(m_chartView->rect());
    }
}

// ============================================================================
// 内部方法 -- Series管理
// ============================================================================

/** @brief 为指定通道创建QLineSeries并添加到图表 @param name 通道名称 @param color 线条颜色 */
void ChartWidget::createSeries(const QString& name, const QColor& color)
{
    if (m_seriesMap.contains(name)) return;

    // 如果没有提供有效颜色，从当前主题调色板中按索引选取
    QColor chColor = color;
    if (!chColor.isValid()) {
        bool isDark = ThemeManager::instance().currentTheme().contains("dark");
        QVector<QColor> palette = ChartColors::colorsForTheme(isDark);
        if (!palette.isEmpty()) {
            chColor = palette[m_seriesMap.size() % palette.size()];
        } else {
            chColor = Qt::cyan;  // 防御性回退: 空调色板时使用默认颜色
        }
    }

    auto* series = new QLineSeries;
    series->setName(name);
    series->setColor(chColor);
    series->setUseOpenGL(true);

    m_chart->addSeries(series);
    series->attachAxis(m_xAxis);
    // 附加到通道对应的独立Y轴（由 YAxisManager 管理）
    m_yAxisManager->attachSeries(name, series);

    m_seriesMap[name] = series;
}

/** @brief 移除指定通道的QLineSeries @param name 通道名称 */
void ChartWidget::removeSeries(const QString& name)
{
    auto it = m_seriesMap.find(name);
    if (it == m_seriesMap.end()) return;

    m_chart->removeSeries(it.value());
    it.value()->deleteLater();
    m_seriesMap.erase(it);
}
