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

#include "chart/ChartWidget.h"
#include "protocol/FrameDefinition.h"
#include "core/ThemeManager.h"

#include <QtCharts>
#include <algorithm>

// ============================================================================
// 构造函数
// ============================================================================

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

void ChartWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ---- 顶部控制栏 ----
    auto* toolbar = new QWidget;
    toolbar->setObjectName("chartToolbar");
    auto* toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(8, 4, 8, 4);

    m_pauseBtn = new QPushButton(tr("暂停"));
    m_pauseBtn->setObjectName("chartPauseBtn");
    m_pauseBtn->setCheckable(true);
    m_pauseBtn->setMinimumWidth(60);

    m_clearBtn = new QPushButton(tr("清除"));
    m_clearBtn->setObjectName("chartClearBtn");
    m_clearBtn->setMinimumWidth(50);

    auto* windowLabel = new QLabel(tr("窗口:"));
    windowLabel->setObjectName("chartWindowLabel");
    toolLayout->addWidget(windowLabel);
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
    toolLayout->addWidget(m_windowSizeCombo);
    toolLayout->addStretch();
    toolLayout->addWidget(m_statusLabel);
    layout->addWidget(toolbar);

    // ---- 图表区域 ----
    m_chart = new QChart;
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setMargins(QMargins(4, 4, 4, 4));

    m_xAxis = new QValueAxis;
    m_xAxis->setTitleText(tr("采样数"));
    m_xAxis->setLabelFormat("%d");
    m_chart->addAxis(m_xAxis, Qt::AlignBottom);

    m_yAxis = new QValueAxis;
    m_yAxis->setTitleText(tr("数值"));
    m_chart->addAxis(m_yAxis, Qt::AlignLeft);

    // 设置初始轴范围，使图表在没有数据时也能渲染背景
    m_xAxis->setRange(0, 10);
    m_yAxis->setRange(0, 100);

    m_chartView = new QChartView(m_chart);
    m_chartView->setObjectName("chartView");  // QSS 选择器需要
    m_chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(m_chartView, 1);

    // ---- 控制栏信号连接 ----
    connect(m_pauseBtn, &QPushButton::toggled, this, &ChartWidget::onPauseToggled);
    connect(m_clearBtn, &QPushButton::clicked, this, &ChartWidget::onClearClicked);
    connect(m_windowSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                int sizes[] = {100, 200, 500, 1000, 2000};
                setWindowSize(sizes[idx]);
            });

    m_statusLabel->setText(tr("通道: 0"));
}

// ============================================================================
// 公开接口
// ============================================================================

ChartModel* ChartWidget::model() const
{
    return m_model;
}

void ChartWidget::configureFromFrameDefinition(const FrameDefinition& def)
{
    // 从帧定义的字段列表自动生成通道配置
    m_configSet = ChannelConfigSet::generateDefaults(def.fields);

    // 应用到ChartModel（会触发 channelsChanged 信号 -> 重建渲染层）
    m_model->setChannelConfigSet(m_configSet);
}

void ChartWidget::setWindowSize(int points)
{
    m_model->setWindowSize(points);
}

void ChartWidget::clear()
{
    m_model->clear();
}

QStringList ChartWidget::channels() const
{
    return m_model->channelNames();
}

void ChartWidget::setYRange(double min, double max)
{
    m_autoYRange = false;
    m_yAxis->setRange(min, max);
}

void ChartWidget::setAutoYRange(bool enabled)
{
    m_autoYRange = enabled;
}

// ============================================================================
// 槽函数 -- 帧数据接收（兼容旧接口，委托给ChartModel）
// ============================================================================

void ChartWidget::onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame)
{
    if (m_paused) return;
    m_model->onFrameParsed(fields, rawFrame);
}

// ============================================================================
// 槽函数 -- ChartModel 信号驱动的渲染更新
// ============================================================================

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

    // 自动Y轴范围
    if (m_autoYRange) {
        QPair<double, double> yRange = m_model->globalYRange();
        if (yRange.first != 0.0 || yRange.second != 0.0) {
            double margin = (yRange.second - yRange.first) * 0.1;
            if (margin < 0.001) margin = 1.0;
            m_yAxis->setRange(yRange.first - margin, yRange.second + margin);
        }
    }

    // 更新状态标签
    m_statusLabel->setText(tr("通道: %1 | 帧: %2")
        .arg(m_seriesMap.size())
        .arg(m_model->currentFrameIndex()));
}

void ChartWidget::onChannelsChanged()
{
    // 清除旧的series
    for (auto* series : m_seriesMap) {
        m_chart->removeSeries(series);
        series->deleteLater();
    }
    m_seriesMap.clear();

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
        for (int i = 0; i < names.size(); ++i) {
            QColor chColor = palette[m_seriesMap.size() % palette.size()];
            createSeries(names[i], chColor);
        }
        m_statusLabel->setText(tr("通道: %1").arg(m_seriesMap.size()));
        return;
    }

    // ---- 主路径: 根据完整的通道配置创建series ----
    const QVector<ChannelConfig>& channels = m_configSet.channels();
    for (const ChannelConfig& cfg : channels) {
        if (cfg.enabled) {
            // 如果通道配置有自定义颜色则使用，否则从主题调色板获取
            QColor chColor = cfg.color.isValid() ? cfg.color :
                palette[m_seriesMap.size() % palette.size()];
            createSeries(cfg.displayName, chColor);
        }
    }

    m_statusLabel->setText(tr("通道: %1").arg(m_seriesMap.size()));
}

void ChartWidget::onDataCleared()
{
    // 清除所有series的数据点
    for (auto* series : m_seriesMap) {
        series->clear();
    }
    m_xAxis->setRange(0, 10);
    m_yAxis->setRange(0, 100);
}

// ============================================================================
// 槽函数 -- 控制栏按钮
// ============================================================================

void ChartWidget::onPauseToggled(bool paused)
{
    m_paused = paused;
    m_pauseBtn->setText(paused ? tr("继续") : tr("暂停"));
}

void ChartWidget::onClearClicked()
{
    clear();
}

// ============================================================================
// 主题切换 -- 响应 ThemeManager::themeChanged 信号
// ============================================================================

/**
 * @brief 主题切换时重绘所有图表视觉元素
 *
 * 调用 applyThemeColors() 更新:
 *   - 图表背景色 (ThemeManager::BgPrimary)
 *   - 网格线颜色 (ThemeManager::Border)
 *   - 坐标轴标签颜色 (ThemeManager::TextSecondary)
 *   - 图例文字颜色 (ThemeManager::TextSecondary)
 *   - 所有数据线颜色 (ChartColors::colorsForTheme)
 */
void ChartWidget::onThemeChanged()
{
    applyThemeColors();
}

/**
 * @brief 应用当前主题颜色到图表所有视觉元素
 *
 * 从 ThemeManager 获取语义色值并应用到:
 *   1. QChart 背景画刷 (BgPrimary)
 *   2. QChart 绘图区域背景 (BgPrimary)
 *   3. X/Y 坐标轴网格线颜色 (Border)
 *   4. X/Y 坐标轴刻度标签颜色 (TextSecondary)
 *   5. X/Y 坐标轴标题颜色 (TextSecondary)
 *   6. 图例标签颜色 (TextSecondary)
 *   7. 所有 QLineSeries 数据线颜色 (ChartColors 主题调色板)
 *
 * 数据线颜色更新策略:
 *   - 主题切换时，所有数据线按通道索引从新调色板中重新分配颜色
 *   - 这确保在暗色/亮色背景下线条都有足够对比度
 */
void ChartWidget::applyThemeColors()
{
    auto& theme = ThemeManager::instance();

    // 判断当前是否为暗色主题（根据主题名称判断）
    QString themeName = theme.currentTheme();
    bool isDark = themeName.contains("dark");

    // ---- 1. 图表背景色 ----
    QColor bgColor = theme.color(ThemeManager::SemanticColor::BgPrimary);
    m_chart->setBackgroundBrush(QBrush(bgColor));
    m_chart->setPlotAreaBackgroundBrush(QBrush(bgColor));
    m_chart->setPlotAreaBackgroundVisible(true);

    // ---- 2. 网格线颜色 ----
    QColor gridColor = theme.color(ThemeManager::SemanticColor::Border);

    // X轴网格线和标签颜色
    m_xAxis->setGridLineColor(gridColor);
    m_xAxis->setLinePen(QPen(gridColor, 1));

    // Y轴网格线和标签颜色
    m_yAxis->setGridLineColor(gridColor);
    m_yAxis->setLinePen(QPen(gridColor, 1));

    // ---- 3. 坐标轴标签颜色 ----
    QColor labelColor = theme.color(ThemeManager::SemanticColor::TextSecondary);

    // X轴标签和标题
    QBrush labelBrush(labelColor);
    m_xAxis->setLabelsBrush(labelBrush);
    m_xAxis->setTitleBrush(labelColor);

    // Y轴标签和标题
    m_yAxis->setLabelsBrush(labelBrush);
    m_yAxis->setTitleBrush(labelColor);

    // ---- 4. 图例文字颜色 ----
    if (m_chart->legend()) {
        m_chart->legend()->setLabelColor(labelColor);
    }

    // ---- 5. 数据线颜色 ----
    // 从当前主题对应的调色板中按通道索引重新分配颜色
    const QVector<QColor>& palette = ChartColors::colorsForTheme(isDark);
    if (palette.isEmpty()) return;  // 防御性检查: 空调色板无法分配颜色
    int index = 0;
    for (auto it = m_seriesMap.begin(); it != m_seriesMap.end(); ++it, ++index) {
        QColor lineColor = palette[index % palette.size()];
        it.value()->setColor(lineColor);
    }
}

// ============================================================================
// 内部方法 -- Series管理
// ============================================================================

void ChartWidget::createSeries(const QString& name, const QColor& color)
{
    if (m_seriesMap.contains(name)) return;

    // 如果没有提供有效颜色，从当前主题调色板中按索引选取
    QColor chColor = color;
    if (!chColor.isValid()) {
        bool isDark = ThemeManager::instance().currentTheme().contains("dark");
        const QVector<QColor>& palette = ChartColors::colorsForTheme(isDark);
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
    series->attachAxis(m_yAxis);

    m_seriesMap[name] = series;
}

void ChartWidget::removeSeries(const QString& name)
{
    auto it = m_seriesMap.find(name);
    if (it == m_seriesMap.end()) return;

    m_chart->removeSeries(it.value());
    it.value()->deleteLater();
    m_seriesMap.erase(it);
}
