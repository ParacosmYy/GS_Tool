#include "ChartWidget.h"
#include <QtCharts>

const QVector<QColor> ChartWidget::kDefaultColors = {
    QColor("#89b4fa"), QColor("#a6e3a1"), QColor("#f9e2af"),
    QColor("#f38ba8"), QColor("#94e2d5"), QColor("#cba6f7"),
    QColor("#fab387"), QColor("#74c7ec"), QColor("#f5c2e7"),
    QColor("#b4befe")
};

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ChartWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 顶部控制栏
    auto* toolbar = new QWidget;
    auto* toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(8, 4, 8, 4);

    m_pauseBtn = new QPushButton(tr("Pause"));
    m_pauseBtn->setCheckable(true);
    m_pauseBtn->setFixedWidth(60);

    m_clearBtn = new QPushButton(tr("Clear"));
    m_clearBtn->setFixedWidth(50);

    toolLayout->addWidget(new QLabel(tr("Window:")));
    m_windowSizeCombo = new QComboBox;
    m_windowSizeCombo->addItems({"100", "200", "500", "1000", "2000"});
    m_windowSizeCombo->setCurrentIndex(1);
    m_windowSizeCombo->setFixedWidth(70);

    m_statusLabel = new QLabel;
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
}

void ChartWidget::addChannel(const QString& name, const QColor& color)
{
    if (m_channels.contains(name)) return;

    QColor chColor = color.isValid() ? color :
        kDefaultColors[m_channels.size() % kDefaultColors.size()];

    auto* series = new QLineSeries;
    series->setName(name);
    series->setColor(chColor);
    series->setUseOpenGL(true); // 使用OpenGL加速渲染

    m_chart->addSeries(series);
    series->attachAxis(m_xAxis);
    series->attachAxis(m_yAxis);

    ChannelData data;
    data.series = series;
    m_channels[name] = data;

    m_statusLabel->setText(tr("Channels: %1").arg(m_channels.size()));
}

void ChartWidget::removeChannel(const QString& name)
{
    auto it = m_channels.find(name);
    if (it == m_channels.end()) return;

    m_chart->removeSeries(it->series);
    it->series->deleteLater();
    m_channels.erase(it);

    m_statusLabel->setText(tr("Channels: %1").arg(m_channels.size()));
}

void ChartWidget::appendData(const QString& channel, double value)
{
    if (m_paused) return;

    auto it = m_channels.find(channel);
    if (it == m_channels.end()) return;

    // 添加数据点
    it->points.append(QPointF(m_xCounter, value));

    // 维护滑动窗口
    while (it->points.size() > m_windowSize) {
        it->points.removeFirst();
    }

    // 更新Y轴范围
    if (m_autoYRange) {
        double globalMin = std::numeric_limits<double>::max();
        double globalMax = std::numeric_limits<double>::lowest();
        for (auto& ch : m_channels) {
            for (const auto& pt : ch.points) {
                if (pt.y() < globalMin) globalMin = pt.y();
                if (pt.y() > globalMax) globalMax = pt.y();
            }
        }
        double margin = (globalMax - globalMin) * 0.1;
        if (margin < 0.001) margin = 1.0;
        m_yAxis->setRange(globalMin - margin, globalMax + margin);
    }

    // 批量更新series（高性能：替换所有点而非逐个添加）
    QVector<QPointF> visiblePoints = it->points;
    it->series->replace(visiblePoints);

    // 每个通道独立计数会不同，但所有通道共享X轴
    // 只在第一个通道增加X计数
    if (channel == m_channels.firstKey()) {
        m_xCounter++;
        // 更新X轴范围
        if (it->points.size() >= m_windowSize) {
            m_xAxis->setRange(m_xCounter - m_windowSize, m_xCounter);
        } else {
            m_xAxis->setRange(0, m_xCounter + 10);
        }
    }
}

void ChartWidget::setWindowSize(int points)
{
    m_windowSize = points;
}

void ChartWidget::clear()
{
    m_xCounter = 0;
    for (auto& ch : m_channels) {
        ch.points.clear();
        ch.series->clear();
        ch.yMin = 0;
        ch.yMax = 0;
    }
    m_xAxis->setRange(0, 10);
    m_yAxis->setRange(0, 100);
}

QStringList ChartWidget::channels() const
{
    return m_channels.keys();
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

void ChartWidget::onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame)
{
    Q_UNUSED(rawFrame);
    // 自动从解析字段中提取数值并添加到对应通道
    for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
        // 跳过内部字段和非数值字段
        if (it.key().startsWith('_')) continue;

        bool ok;
        double value = it.value().toDouble(&ok);
        if (!ok) continue;

        // 如果通道不存在，自动创建
        if (!m_channels.contains(it.key())) {
            addChannel(it.key());
        }
        appendData(it.key(), value);
    }
}

void ChartWidget::onPauseToggled(bool paused)
{
    m_paused = paused;
    m_pauseBtn->setText(paused ? tr("Resume") : tr("Pause"));
}

void ChartWidget::onClearClicked()
{
    clear();
}
