/**
 * @file ScatterWidget.cpp
 * @brief X/Y散点图控件实现 -- 双通道数据相关性分析面板
 *
 * 实现ScatterWidget的UI布局、信号连接、散点绘制和Pearson相关系数计算。
 * 从ChartModel读取两个指定通道的数据，使用QScatterSeries绘制散点图。
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
#include <cmath>
#include <algorithm>

// ============================================================
// 构造 / 初始化
// ============================================================

/**
 * @brief 构造散点图控件
 *
 * 初始化UI、连接ChartModel和ThemeManager信号、应用初始主题。
 * 遵循FftWidget的构造模式。
 */
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

/** @brief 创建顶部工具栏 — X/Y通道选择、刷新按钮、自动刷新 */
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

// ============================================================
// 散点绘制与相关系数计算
// ============================================================

/**
 * @brief 刷新散点图显示
 *
 * 从ChartModel读取X/Y通道数据，取两者最小长度，
 * 以X通道值为横坐标、Y通道值为纵坐标绘制散点。
 * 同时计算Pearson相关系数并更新标签。
 */
void ScatterWidget::refreshPlot()
{
    if (!m_model || !m_series) {
        return;
    }

    // 获取当前选中的通道名
    QString xName = m_xChannelCombo->currentText();
    QString yName = m_yChannelCombo->currentText();
    if (xName.isEmpty() || yName.isEmpty()) {
        m_series->replace({});
        m_correlationLabel->setText(tr("数据不足"));
        ++m_totalClears;
        return;
    }

    // 读取通道数据
    QVector<QPointF> xData = m_model->channelData(xName);
    QVector<QPointF> yData = m_model->channelData(yName);

    int n = qMin(xData.size(), yData.size());
    if (n == 0) {
        m_series->replace({});
        m_correlationLabel->setText(tr("数据不足"));
        ++m_totalClears;
        return;
    }

    // 构造散点: x = X通道值的y分量, y = Y通道值的y分量
    QVector<QPointF> points;
    points.reserve(n);
    double xMin = xData[0].y(), xMax = xData[0].y();
    double yMin = yData[0].y(), yMax = yData[0].y();

    for (int i = 0; i < n; ++i) {
        double xv = xData[i].y();
        double yv = yData[i].y();
        points.append(QPointF(xv, yv));
        if (xv < xMin) xMin = xv;
        if (xv > xMax) xMax = xv;
        if (yv < yMin) yMin = yv;
        if (yv > yMax) yMax = yv;
    }

    m_series->replace(points);
    m_totalPointsPlotted += n;

    // 自动调整坐标轴范围（留5%余量）
    double xPad = qMax((xMax - xMin) * 0.05, 0.001);
    double yPad = qMax((yMax - yMin) * 0.05, 0.001);
    m_xAxis->setRange(xMin - xPad, xMax + xPad);
    m_yAxis->setRange(yMin - yPad, yMax + yPad);

    // 更新轴标题
    m_xAxis->setTitleText(tr("X: %1").arg(xName));
    m_yAxis->setTitleText(tr("Y: %1").arg(yName));

    // 计算Pearson相关系数
    double r = computePearsonCorrelation(xData, yData);
    if (std::isnan(r)) {
        m_correlationLabel->setText(tr("数据不足"));
    } else {
        m_correlationLabel->setText(
            tr("相关系数 r = %1  |  样本数: %2")
                .arg(r, 0, 'f', 4)
                .arg(n));
    }
}

/**
 * @brief 计算Pearson相关系数
 *
 * 标准公式: r = Σ((x_i - x̄)(y_i - ȳ)) / sqrt(Σ(x_i-x̄)² × Σ(y_i-ȳ)²)
 * 取两组数据的最小长度进行配对计算。
 *
 * @param xData X轴数据（取QPointF的y分量作为值）
 * @param yData Y轴数据（取QPointF的y分量作为值）
 * @return Pearson r，数据不足或零方差时返回NaN
 */
double ScatterWidget::computePearsonCorrelation(
    const QVector<QPointF>& xData, const QVector<QPointF>& yData)
{
    int n = qMin(xData.size(), yData.size());
    if (n < 2) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    // 计算均值
    double xSum = 0.0, ySum = 0.0;
    for (int i = 0; i < n; ++i) {
        xSum += xData[i].y();
        ySum += yData[i].y();
    }
    double xMean = xSum / n;
    double yMean = ySum / n;

    // 计算协方差和方差
    double covXY = 0.0, varX = 0.0, varY = 0.0;
    for (int i = 0; i < n; ++i) {
        double dx = xData[i].y() - xMean;
        double dy = yData[i].y() - yMean;
        covXY += dx * dy;
        varX  += dx * dx;
        varY  += dy * dy;
    }

    // 零方差检查
    if (varX == 0.0 || varY == 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return covXY / std::sqrt(varX * varY);
}

// ============================================================
// 槽函数
// ============================================================

/** @brief X轴通道变更时自动刷新 */
void ScatterWidget::onXChannelChanged(int /*index*/)
{
    if (m_autoRefresh) {
        refreshPlot();
    }
}

/** @brief Y轴通道变更时自动刷新 */
void ScatterWidget::onYChannelChanged(int /*index*/)
{
    if (m_autoRefresh) {
        refreshPlot();
    }
}

/** @brief 自动刷新开关切换 */
void ScatterWidget::onAutoRefreshToggled(bool checked)
{
    m_autoRefresh = checked;
}

/**
 * @brief ChartModel数据更新回调
 *
 * 仅在自动刷新模式下，且当前X或Y通道有数据更新时触发重绘。
 */
void ScatterWidget::onDataUpdated(const QStringList& updatedChannels)
{
    if (!m_autoRefresh) {
        return;
    }

    QString xName = m_xChannelCombo->currentText();
    QString yName = m_yChannelCombo->currentText();
    if (updatedChannels.contains(xName) || updatedChannels.contains(yName)) {
        refreshPlot();
    }
}

/**
 * @brief 通道列表变更时重建X/Y下拉框
 *
 * 保存之前的选择，重建列表后尽量恢复。
 */
void ScatterWidget::onChannelsChanged()
{
    if (!m_model) {
        return;
    }

    // 保存当前选择
    QString prevX = m_xChannelCombo->currentText();
    QString prevY = m_yChannelCombo->currentText();

    // 重建X通道下拉框
    m_xChannelCombo->blockSignals(true);
    m_xChannelCombo->clear();
    QStringList names = m_model->channelNames();
    for (const auto& name : names) {
        m_xChannelCombo->addItem(name);
    }
    int xIdx = m_xChannelCombo->findText(prevX);
    if (xIdx >= 0) {
        m_xChannelCombo->setCurrentIndex(xIdx);
    } else if (m_xChannelCombo->count() > 0) {
        m_xChannelCombo->setCurrentIndex(0);
    }
    m_xChannelCombo->blockSignals(false);

    // 重建Y通道下拉框
    m_yChannelCombo->blockSignals(true);
    m_yChannelCombo->clear();
    for (const auto& name : names) {
        m_yChannelCombo->addItem(name);
    }
    int yIdx = m_yChannelCombo->findText(prevY);
    if (yIdx >= 0) {
        m_yChannelCombo->setCurrentIndex(yIdx);
    } else if (m_yChannelCombo->count() > 0) {
        m_yChannelCombo->setCurrentIndex(
            qMin(1, m_yChannelCombo->count() - 1));
    }
    m_yChannelCombo->blockSignals(false);

    // 自动刷新散点图
    if (m_autoRefresh && m_xChannelCombo->count() > 0) {
        refreshPlot();
    }
}

/** @brief 主题切换响应 */
void ScatterWidget::onThemeChanged()
{
    applyThemeColors();
}

// ============================================================
// 主题样式
// ============================================================

/**
 * @brief 应用当前主题颜色到图表
 *
 * 更新内容:
 *   - 图表背景色 (BgPrimary)
 *   - 网格线颜色 (Border)
 *   - 坐标轴标签颜色 (TextSecondary)
 *   - 散点颜色 (ChartColors调色板第一色)
 */
void ScatterWidget::applyThemeColors()
{
    auto& theme = ThemeManager::instance();

    // 图表背景
    QColor bgColor = theme.color(ThemeManager::SemanticColor::BgPrimary);
    m_chart->setBackgroundBrush(bgColor);

    // 网格线和坐标轴颜色
    QColor gridColor = theme.color(ThemeManager::SemanticColor::Border);
    QColor labelColor = theme.color(ThemeManager::SemanticColor::TextSecondary);

    // X轴样式
    m_xAxis->setLinePen(QPen(gridColor, 1));
    m_xAxis->setGridLinePen(QPen(gridColor, 1, Qt::DashLine));
    m_xAxis->setLabelsBrush(labelColor);
    m_xAxis->setTitleBrush(labelColor);

    // Y轴样式
    m_yAxis->setLinePen(QPen(gridColor, 1));
    m_yAxis->setGridLinePen(QPen(gridColor, 1, Qt::DashLine));
    m_yAxis->setLabelsBrush(labelColor);
    m_yAxis->setTitleBrush(labelColor);

    // 散点颜色: 使用ChartColors调色板第一色
    bool isDark = (theme.currentTheme().startsWith(QStringLiteral("dark")));
    const auto& colors = ChartColors::colorsForTheme(isDark);
    QColor dotColor = colors.isEmpty() ? QColor("#89b4fa") : colors.first();
    m_series->setColor(dotColor);
    m_series->setBorderColor(dotColor);

    // 绘图区背景
    m_chart->setPlotAreaBackgroundBrush(QBrush(bgColor));
    m_chart->setPlotAreaBackgroundVisible(true);
}

/** @brief 获取累计绘制点数 */
quint64 ScatterWidget::totalPointsPlotted() const
{
    return m_totalPointsPlotted;
}

/** @brief 获取累计清除次数 */
quint64 ScatterWidget::totalClears() const
{
    return m_totalClears;
}

/** @brief 重置所有散点图统计计数器 */
void ScatterWidget::resetScatterStatistics()
{
    m_totalPointsPlotted = 0;
    m_totalClears = 0;
}
