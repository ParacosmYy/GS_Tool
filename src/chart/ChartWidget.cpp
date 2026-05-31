#include "chart/ChartWidget.h"
#include "protocol/FrameDefinition.h"

#include <QtCharts>
#include <algorithm>

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
    , m_model(new ChartModel(this))
{
    setupUI();

    // 连接 ChartModel 信号到渲染槽
    connect(m_model, &ChartModel::dataUpdated,
            this, &ChartWidget::updateChart);
    connect(m_model, &ChartModel::channelsChanged,
            this, &ChartWidget::onChannelsChanged);
    connect(m_model, &ChartModel::dataCleared,
            this, &ChartWidget::onDataCleared);
}

void ChartWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 顶部控制栏
    auto* toolbar = new QWidget;
    toolbar->setObjectName("chartToolbar");
    auto* toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(8, 4, 8, 4);

    m_pauseBtn = new QPushButton(tr("Pause"));
    m_pauseBtn->setObjectName("chartPauseBtn");
    m_pauseBtn->setCheckable(true);
    m_pauseBtn->setMinimumWidth(60);

    m_clearBtn = new QPushButton(tr("Clear"));
    m_clearBtn->setObjectName("chartClearBtn");
    m_clearBtn->setMinimumWidth(50);

    toolLayout->addWidget(new QLabel(tr("Window:")));
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

    // 图表
    m_chart = new QChart;
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setMargins(QMargins(4, 4, 4, 4));

    m_xAxis = new QValueAxis;
    m_xAxis->setTitleText(tr("Samples"));
    m_xAxis->setLabelFormat("%d");
    m_chart->addAxis(m_xAxis, Qt::AlignBottom);

    m_yAxis = new QValueAxis;
    m_yAxis->setTitleText(tr("Value"));
    m_chart->addAxis(m_yAxis, Qt::AlignLeft);

    // 设置初始轴范围，使图表在没有数据时也能渲染背景
    m_xAxis->setRange(0, 10);
    m_yAxis->setRange(0, 100);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(m_chartView, 1);

    // 信号连接
    connect(m_pauseBtn, &QPushButton::toggled, this, &ChartWidget::onPauseToggled);
    connect(m_clearBtn, &QPushButton::clicked, this, &ChartWidget::onClearClicked);
    connect(m_windowSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                int sizes[] = {100, 200, 500, 1000, 2000};
                setWindowSize(sizes[idx]);
            });

    m_statusLabel->setText(tr("Channels: 0"));
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

    // 应用到ChartModel（会触发 channelsChanged 信号 → 重建渲染层）
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
    m_statusLabel->setText(tr("Channels: %1 | Frames: %2")
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

    // 根据新的通道配置创建series
    const QVector<ChannelConfig>& channels = m_configSet.channels();
    for (const ChannelConfig& cfg : channels) {
        if (cfg.enabled) {
            createSeries(cfg.displayName, cfg.color);
        }
    }

    m_statusLabel->setText(tr("Channels: %1").arg(m_seriesMap.size()));
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
    m_pauseBtn->setText(paused ? tr("Resume") : tr("Pause"));
}

void ChartWidget::onClearClicked()
{
    clear();
}

// ============================================================================
// 内部方法 -- Series管理
// ============================================================================

void ChartWidget::createSeries(const QString& name, const QColor& color)
{
    if (m_seriesMap.contains(name)) return;

    QColor chColor = color.isValid() ? color :
        ChartColors::defaultColors()[m_seriesMap.size() % ChartColors::defaultColors().size()];

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
