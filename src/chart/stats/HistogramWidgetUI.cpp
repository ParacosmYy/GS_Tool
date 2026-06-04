/**
 * @file HistogramWidgetUI.cpp
 * @brief 直方图控件UI布局、工具栏、图表创建及主题颜色应用
 *
 * 从HistogramWidget.cpp拆分而来，包含:
 *   - setupUI(): 整体布局初始化(工具栏+图表+统计标签)
 *   - createToolbar(): 顶部配置工具栏(通道选择/分桶数/刷新/自动刷新)
 *   - setupChart(): QChartView + QBarSeries + 双轴创建
 *   - applyThemeColors(): 主题颜色应用到图表背景/网格/标签/柱体
 */

#include "chart/stats/HistogramWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QtCharts>

#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

/** @brief 初始化整体布局 -- 工具栏+图表+统计摘要标签 */
void HistogramWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);
    mainLayout->addWidget(createToolbar());

    setupChart();
    mainLayout->addWidget(m_chartView, 1);

    m_statsLabel = new QLabel(this);
    m_statsLabel->setObjectName(QStringLiteral("HistogramStatsLabel"));
    m_statsLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_statsLabel->setWordWrap(true);
    mainLayout->addWidget(m_statsLabel);

    setLayout(mainLayout);
}

/** @brief 创建顶部工具栏 -- 通道选择、分桶数、刷新按钮、自动刷新开关 @return 工具栏Widget指针 */
QWidget* HistogramWidget::createToolbar()
{
    auto* toolbar = new QWidget(this);
    toolbar->setObjectName(QStringLiteral("HistogramToolbar"));
    auto* layout = new QHBoxLayout(toolbar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* channelLabel = new QLabel(tr("通道:"), toolbar);
    channelLabel->setObjectName(QStringLiteral("HistogramChannelLabel"));
    m_channelCombo = new QComboBox(toolbar);
    m_channelCombo->setObjectName(QStringLiteral("HistogramChannelCombo"));
    m_channelCombo->setMinimumWidth(100);
    connect(m_channelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HistogramWidget::onChannelChanged);

    auto* binsLabel = new QLabel(tr("分桶数:"), toolbar);
    binsLabel->setObjectName(QStringLiteral("HistogramBinsLabel"));
    m_binsSpin = new QSpinBox(toolbar);
    m_binsSpin->setObjectName(QStringLiteral("HistogramBinsSpin"));
    m_binsSpin->setRange(5, 200);
    m_binsSpin->setValue(30);
    connect(m_binsSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &HistogramWidget::onBinsChanged);

    m_refreshBtn = new QPushButton(tr("刷新"), toolbar);
    m_refreshBtn->setObjectName(QStringLiteral("HistogramRefreshBtn"));
    connect(m_refreshBtn, &QPushButton::clicked,
            this, &HistogramWidget::refreshHistogram);

    m_autoRefreshCheck = new QCheckBox(tr("自动刷新"), toolbar);
    m_autoRefreshCheck->setObjectName(QStringLiteral("HistogramAutoRefreshCheck"));
    m_autoRefreshCheck->setChecked(m_autoRefresh);
    connect(m_autoRefreshCheck, &QCheckBox::toggled,
            this, &HistogramWidget::onAutoRefreshToggled);

    layout->addWidget(channelLabel);
    layout->addWidget(m_channelCombo);
    layout->addWidget(binsLabel);
    layout->addWidget(m_binsSpin);
    layout->addWidget(m_refreshBtn);
    layout->addWidget(m_autoRefreshCheck);
    layout->addStretch();
    return toolbar;
}

/** @brief 创建QChartView + QBarSeries + QBarCategoryAxis/QValueAxis双轴 */
void HistogramWidget::setupChart()
{
    m_chart = new QChart();
    m_chart->setObjectName(QStringLiteral("HistogramChart"));
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(2, 2, 2, 2));

    m_barSet = new QBarSet(QStringLiteral(""), m_chart);
    m_barSet->setObjectName(QStringLiteral("HistogramBarSet"));
    m_series = new QBarSeries(m_chart);
    m_series->setObjectName(QStringLiteral("HistogramBarSeries"));
    m_series->append(m_barSet);
    m_series->setBarWidth(1.0);
    m_chart->addSeries(m_series);

    m_xAxis = new QBarCategoryAxis(m_chart);
    m_xAxis->setObjectName(QStringLiteral("HistogramXAxis"));
    m_xAxis->setTitleText(tr("数值"));
    m_xAxis->setLabelsAngle(-45);

    m_yAxis = new QValueAxis(m_chart);
    m_yAxis->setObjectName(QStringLiteral("HistogramYAxis"));
    m_yAxis->setTitleText(tr("计数"));
    m_yAxis->setLabelFormat(QStringLiteral("%d"));
    m_yAxis->setRange(0, 1);

    m_chart->addAxis(m_xAxis, Qt::AlignBottom);
    m_chart->addAxis(m_yAxis, Qt::AlignLeft);
    m_series->attachAxis(m_xAxis);
    m_series->attachAxis(m_yAxis);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setObjectName(QStringLiteral("HistogramChartView"));
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

/** @brief 应用当前主题颜色到图表背景/网格线/坐标轴标签/柱体颜色 */
void HistogramWidget::applyThemeColors()
{
    auto& theme = ThemeManager::instance();

    // 图表背景
    QColor bgColor = theme.color(ThemeManager::SemanticColor::BgPrimary);
    m_chart->setBackgroundBrush(bgColor);

    // 网格线和坐标轴颜色
    QColor gridColor = theme.color(ThemeManager::SemanticColor::Border);
    QColor labelColor = theme.color(ThemeManager::SemanticColor::TextSecondary);

    m_xAxis->setLinePen(QPen(gridColor, 1));
    m_xAxis->setLabelsBrush(labelColor);
    m_xAxis->setTitleBrush(labelColor);

    m_yAxis->setLinePen(QPen(gridColor, 1));
    m_yAxis->setGridLinePen(QPen(gridColor, 1, Qt::DashLine));
    m_yAxis->setLabelsBrush(labelColor);
    m_yAxis->setTitleBrush(labelColor);

    // 柱体颜色: 使用ChartColors调色板第一色
    bool isDark = (theme.currentTheme().startsWith(QStringLiteral("dark")));
    const auto& colors = ChartColors::colorsForTheme(isDark);
    QColor barColor = colors.isEmpty() ? QColor("#89b4fa") : colors.first();
    m_barSet->setColor(barColor);
    m_barSet->setBorderColor(barColor.darker(120));

    // 绘图区背景
    m_chart->setPlotAreaBackgroundBrush(QBrush(bgColor));
    m_chart->setPlotAreaBackgroundVisible(true);
}
