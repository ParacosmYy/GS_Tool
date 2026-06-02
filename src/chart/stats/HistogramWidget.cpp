/**
 * @file HistogramWidget.cpp
 * @brief 统计直方图控件实现 -- 通道数据值分布分析面板
 *
 * 实现HistogramWidget的UI布局、信号连接、直方图计算和渲染逻辑。
 * 从ChartModel读取通道数据，计算值分布直方图，
 * 使用QBarSeries绘制直方图柱状图，下方显示统计摘要。
 */

#include "chart/stats/HistogramWidget.h"
#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QtCharts>
#include <algorithm>
#include <cmath>

HistogramWidget::HistogramWidget(ChartModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
    , m_chartView(nullptr)
    , m_chart(nullptr)
    , m_series(nullptr)
    , m_barSet(nullptr)
    , m_xAxis(nullptr)
    , m_yAxis(nullptr)
{
    setObjectName(QStringLiteral("HistogramWidget"));
    setupUI();

    if (m_model) {
        connect(m_model, &ChartModel::dataUpdated,
                this, &HistogramWidget::onDataUpdated);
        connect(m_model, &ChartModel::channelsChanged,
                this, &HistogramWidget::onChannelsChanged);
    }
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &HistogramWidget::onThemeChanged);

    applyThemeColors();
    onChannelsChanged();
}

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
    m_binsSpin->setRange(10, 200);
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

QVector<QPair<double, int>> HistogramWidget::computeHistogram(
    const QVector<QPointF>& data, int bins)
{
    QVector<QPair<double, int>> result;
    if (data.isEmpty() || bins <= 0) {
        return result;
    }

    double minVal = data.first().y();
    double maxVal = data.first().y();
    for (const auto& pt : data) {
        minVal = std::min(minVal, pt.y());
        maxVal = std::max(maxVal, pt.y());
    }

    double range = maxVal - minVal;
    if (range < 1e-12) {
        result.append(qMakePair(minVal, data.size()));
        return result;
    }

    double binWidth = range / bins;
    result.resize(bins);
    for (int i = 0; i < bins; ++i) {
        result[i] = qMakePair(minVal + (i + 0.5) * binWidth, 0);
    }

    for (const auto& pt : data) {
        int idx = static_cast<int>((pt.y() - minVal) / binWidth);
        idx = qBound(0, idx, bins - 1);
        result[idx].second++;
    }
    return result;
}

HistogramWidget::Stats HistogramWidget::computeStats(
    const QVector<QPointF>& data)
{
    Stats s{};
    if (data.isEmpty()) {
        return s;
    }

    s.count = data.size();
    s.min = data.first().y();
    s.max = data.first().y();
    double sum = 0.0;
    for (const auto& pt : data) {
        double val = pt.y();
        sum += val;
        s.min = std::min(s.min, val);
        s.max = std::max(s.max, val);
    }
    s.mean = sum / s.count;

    double sqSum = 0.0;
    for (const auto& pt : data) {
        double diff = pt.y() - s.mean;
        sqSum += diff * diff;
    }
    s.stddev = std::sqrt(sqSum / s.count);
    return s;
}

void HistogramWidget::refreshHistogram()
{
    if (!m_model || !m_barSet) {
        return;
    }

    QString channel = m_channelCombo->currentText();
    if (channel.isEmpty()) {
        m_statsLabel->setText(tr("无通道数据"));
        return;
    }

    QVector<QPointF> data = m_model->channelData(channel);
    if (data.isEmpty()) {
        m_barSet->remove(0, m_barSet->count());
        m_statsLabel->setText(tr("无数据"));
        return;
    }

    int bins = m_binsSpin->value();
    auto histData = computeHistogram(data, bins);

    // 更新柱体数据
    m_barSet->remove(0, m_barSet->count());
    QStringList categories;
    for (const auto& [center, count] : histData) {
        *m_barSet << count;
        categories.append(QStringLiteral("%1").arg(center, 0, 'f', 2));
    }
    m_xAxis->setCategories(categories);

    // 自动调整Y轴
    int maxCount = 0;
    for (const auto& [center, count] : histData) {
        maxCount = std::max(maxCount, count);
    }
    m_yAxis->setRange(0, qMax(maxCount + 1, 1));

    // 更新统计摘要
    Stats s = computeStats(data);
    m_statsLabel->setText(
        tr("均值: %1 | 标准差: %2 | 最小: %3 | 最大: %4 | 样本: %5")
            .arg(s.mean, 0, 'f', 3)
            .arg(s.stddev, 0, 'f', 3)
            .arg(s.min, 0, 'f', 3)
            .arg(s.max, 0, 'f', 3)
            .arg(s.count));
}

void HistogramWidget::onChannelChanged(int /*index*/)
{
    if (m_autoRefresh) { refreshHistogram(); }
}

void HistogramWidget::onBinsChanged(int /*value*/)
{
    if (m_autoRefresh) { refreshHistogram(); }
}

void HistogramWidget::onAutoRefreshToggled(bool checked)
{
    m_autoRefresh = checked;
}

void HistogramWidget::onDataUpdated(const QStringList& updatedChannels)
{
    if (!m_autoRefresh) { return; }
    if (updatedChannels.contains(m_channelCombo->currentText())) {
        refreshHistogram();
    }
}

void HistogramWidget::onChannelsChanged()
{
    if (!m_model) { return; }

    QString prevChannel = m_channelCombo->currentText();
    m_channelCombo->blockSignals(true);
    m_channelCombo->clear();
    for (const auto& name : m_model->channelNames()) {
        m_channelCombo->addItem(name);
    }

    int idx = m_channelCombo->findText(prevChannel);
    if (idx >= 0) {
        m_channelCombo->setCurrentIndex(idx);
    } else if (m_channelCombo->count() > 0) {
        m_channelCombo->setCurrentIndex(0);
    }
    m_channelCombo->blockSignals(false);

    if (m_autoRefresh && m_channelCombo->count() > 0) {
        refreshHistogram();
    }
}

void HistogramWidget::onThemeChanged() { applyThemeColors(); }

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
