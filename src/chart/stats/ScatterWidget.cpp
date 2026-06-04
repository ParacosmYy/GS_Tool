/**
 * @file ScatterWidget.cpp
 * @brief X/Y散点图控件实现 -- 构造、UI布局与初始化
 *
 * 实现ScatterWidget的构造函数、UI搭建（工具栏/图表/布局）和信号连接。
 * 散点绘制、相关系数计算、槽函数、主题样式和统计接口见 ScatterWidgetStats.cpp。
 */

#include "chart/stats/ScatterWidget.h"
#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QtCharts>

// ============================================================
// 构造 / 初始化
// ============================================================

/** @brief 构造散点图控件，初始化UI、连接ChartModel和ThemeManager信号、应用初始主题 @param model 数据模型指针 @param parent 父控件 */
ScatterWidget::ScatterWidget(ChartModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
    , m_chartView(nullptr)
    , m_chart(nullptr)
    , m_series(nullptr)
    , m_xAxis(nullptr)
    , m_yAxis(nullptr)
    , m_xChannelCombo(nullptr)
    , m_yChannelCombo(nullptr)
    , m_refreshBtn(nullptr)
    , m_autoRefreshCheck(nullptr)
    , m_correlationLabel(nullptr)
{
    setObjectName(QStringLiteral("ScatterWidget"));

    setupUI();

    // 监听ChartModel信号
    if (m_model) {
        connect(m_model, &ChartModel::dataUpdated,
                this, &ScatterWidget::onDataUpdated);
        connect(m_model, &ChartModel::channelsChanged,
                this, &ScatterWidget::onChannelsChanged);
    }

    // 监听主题切换
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &ScatterWidget::onThemeChanged);

    // 初始主题应用
    applyThemeColors();

    // 初始化通道列表
    onChannelsChanged();
}

// ============================================================
// UI搭建
// ============================================================

/** @brief 初始化整体布局 — 工具栏+图表+相关系数标签 */
void ScatterWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // 工具栏
    mainLayout->addWidget(createToolbar());

    // 图表区域
    setupChart();
    mainLayout->addWidget(m_chartView, 1);

    // 底部相关系数标签
    m_correlationLabel = new QLabel(tr("数据不足"), this);
    m_correlationLabel->setObjectName(QStringLiteral("ScatterCorrelationLabel"));
    m_correlationLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_correlationLabel);

    setLayout(mainLayout);
}

/** @brief 创建顶部工具栏 — X/Y通道选择、刷新按钮、自动刷新 @return 工具栏Widget指针 */
QWidget* ScatterWidget::createToolbar()
{
    auto* toolbar = new QWidget(this);
    toolbar->setObjectName(QStringLiteral("ScatterToolbar"));
    auto* layout = new QHBoxLayout(toolbar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    // X轴通道选择
    auto* xLabel = new QLabel(tr("X通道:"), toolbar);
    xLabel->setObjectName(QStringLiteral("ScatterXLabel"));
    m_xChannelCombo = new QComboBox(toolbar);
    m_xChannelCombo->setObjectName(QStringLiteral("ScatterXCombo"));
    m_xChannelCombo->setMinimumWidth(100);
    connect(m_xChannelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScatterWidget::onXChannelChanged);

    // Y轴通道选择
    auto* yLabel = new QLabel(tr("Y通道:"), toolbar);
    yLabel->setObjectName(QStringLiteral("ScatterYLabel"));
    m_yChannelCombo = new QComboBox(toolbar);
    m_yChannelCombo->setObjectName(QStringLiteral("ScatterYCombo"));
    m_yChannelCombo->setMinimumWidth(100);
    connect(m_yChannelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScatterWidget::onYChannelChanged);

    // 刷新按钮
    m_refreshBtn = new QPushButton(tr("刷新"), toolbar);
    m_refreshBtn->setObjectName(QStringLiteral("ScatterRefreshBtn"));
    connect(m_refreshBtn, &QPushButton::clicked,
            this, &ScatterWidget::refreshPlot);

    // 自动刷新
    m_autoRefreshCheck = new QCheckBox(tr("自动刷新"), toolbar);
    m_autoRefreshCheck->setObjectName(QStringLiteral("ScatterAutoRefreshCheck"));
    m_autoRefreshCheck->setChecked(m_autoRefresh);
    connect(m_autoRefreshCheck, &QCheckBox::toggled,
            this, &ScatterWidget::onAutoRefreshToggled);

    // 布局
    layout->addWidget(xLabel);
    layout->addWidget(m_xChannelCombo);
    layout->addWidget(yLabel);
    layout->addWidget(m_yChannelCombo);
    layout->addWidget(m_refreshBtn);
    layout->addWidget(m_autoRefreshCheck);
    layout->addStretch();

    return toolbar;
}

/** @brief 创建QChartView + QScatterSeries + 双QValueAxis */
void ScatterWidget::setupChart()
{
    m_chart = new QChart();
    m_chart->setObjectName(QStringLiteral("ScatterChart"));
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(2, 2, 2, 2));

    // 散点序列
    m_series = new QScatterSeries(m_chart);
    m_series->setObjectName(QStringLiteral("ScatterSeries"));
    m_series->setMarkerSize(4.0);
    m_series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    m_chart->addSeries(m_series);

    // X轴
    m_xAxis = new QValueAxis(m_chart);
    m_xAxis->setObjectName(QStringLiteral("ScatterXAxis"));
    m_xAxis->setTitleText(tr("X"));
    m_xAxis->setLabelFormat(QStringLiteral("%g"));
    m_xAxis->setRange(0, 1);

    // Y轴
    m_yAxis = new QValueAxis(m_chart);
    m_yAxis->setObjectName(QStringLiteral("ScatterYAxis"));
    m_yAxis->setTitleText(tr("Y"));
    m_yAxis->setLabelFormat(QStringLiteral("%g"));
    m_yAxis->setRange(0, 1);

    m_chart->addAxis(m_xAxis, Qt::AlignBottom);
    m_chart->addAxis(m_yAxis, Qt::AlignLeft);
    m_series->attachAxis(m_xAxis);
    m_series->attachAxis(m_yAxis);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setObjectName(QStringLiteral("ScatterChartView"));
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

// 散点绘制、相关系数计算见 ScatterWidgetStats.cpp
