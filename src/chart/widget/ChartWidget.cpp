/**
 * @file ChartWidget.cpp
 * @brief 实时波形图控件实现
 * 实现要点: 构造连接信号 / 主题切换更新视觉 / ChartColors主题调色板 / 数据线颜色随主题变化
 */

#include "chart/widget/ChartWidget.h"
#include "protocol/parser/FrameDefinition.h"
#include "core/theme/ThemeManager.h"
#include "core/widgets/AnimatedButton.h"

#include <QtCharts>
#include <QPainter>

// ============================================================================
// 构造函数
// ============================================================================

/** @brief 构造波形图控件，创建ChartModel、初始化UI并连接信号 @param parent 父控件 */
ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
    , m_model(new ChartModel(this))
{
    setObjectName("chartWidget");

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

/** @brief 初始化波形图UI，包含QChartView、工具栏、游标叠加层和缩放控制器 */
void ChartWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ---- 顶部控制栏 ----
    layout->addWidget(createToolbar());

    // ---- 图表区域 ----
    m_chart = new QChart;
    m_chart->setObjectName("chartChart");
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setMargins(QMargins(4, 4, 4, 4));

    m_xAxis = new QValueAxis;
    m_xAxis->setObjectName("chartXAxis");
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
    // 统计缩放/平移事件次数
    connect(m_zoomController, &ZoomController::viewChanged, this, [this]() {
        ++m_totalZoomEvents;
        ++m_totalManualZooms;
    });
    connect(m_zoomController, &ZoomController::zoomReset, this, [this]() {
        ++m_totalZoomEvents;
    });
}

/** @brief 创建顶部工具栏，包含暂停/清除/游标/窗口大小/状态标签 @return 工具栏Widget指针 */
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

/** @brief 从帧定义配置波形图，自动创建通道映射 @param def 帧定义 */
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

/** @brief 设置Y轴固定范围，禁用自动Y轴 @param min 最小值 @param max 最大值 */
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

/** @brief 帧解析回调，转发到ChartModel @param fields 字段映射 @param rawFrame 原始帧 */
void ChartWidget::onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame)
{
    if (m_paused) return;
    ++m_totalDataUpdates;
    ++m_totalSamplesAppended;
    m_model->onFrameParsed(fields, rawFrame);
}

// ============================================================================
// 槽函数 -- 控制栏按钮
// ============================================================================

/** @brief 暂停/继续按钮切换回调 @param paused true=暂停 */
void ChartWidget::onPauseToggled(bool paused)
{
    ++m_totalInteractions;
    m_paused = paused;
    m_pauseBtn->setText(paused ? tr("继续") : tr("暂停"));
}

/** @brief 清除按钮回调，清空波形数据和series */
void ChartWidget::onClearClicked()
{
    ++m_totalInteractions;
    clear();
}

// ---- 导出/快照/统计接口已拆分至 ChartWidgetExport.cpp ----
// ---- 主题切换/Series管理已拆分至 ChartWidgetTheme.cpp ----
// ---- 数据渲染/通道同步已拆分至 ChartWidgetRender.cpp ----
