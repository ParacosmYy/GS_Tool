/**
 * @file FftWidget.cpp
 * @brief FFT频谱显示控件实现 -- 主题感知的频谱分析面板
 *
 * 实现FftWidget的UI布局、信号连接、频谱计算和渲染逻辑。
 * 从ChartModel读取通道时域数据，经FftEngine计算FFT后，
 * 使用QLineSeries绘制频率-幅度频谱图。
 */

#include "chart/fft/FftWidget.h"
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

// ============================================================
// 构造 / 初始化
// ============================================================

FftWidget::FftWidget(ChartModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
    , m_engine(new FftEngine(this))
    , m_chartView(nullptr)
    , m_chart(nullptr)
    , m_spectrumSeries(nullptr)
    , m_xAxis(nullptr)
    , m_yAxis(nullptr)
{
    setObjectName(QStringLiteral("FftWidget"));

    setupUI();

    // 监听ChartModel信号
    if (m_model) {
        connect(m_model, &ChartModel::dataUpdated,
                this, &FftWidget::onDataUpdated);
        connect(m_model, &ChartModel::channelsChanged,
                this, &FftWidget::onChannelsChanged);
    }

    // 监听主题切换
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &FftWidget::onThemeChanged);

    // 初始主题应用
    applyThemeColors();

    // 初始化通道列表
    onChannelsChanged();
}

// ============================================================
// 公共接口
// ============================================================

void FftWidget::setSampleRate(double rate)
{
    m_sampleRate = (rate > 0.0) ? rate : 1000.0;
    if (m_sampleRateSpin) {
        m_sampleRateSpin->setValue(static_cast<int>(m_sampleRate));
    }
}

double FftWidget::sampleRate() const
{
    return m_sampleRate;
}

// ============================================================
// UI搭建
// ============================================================

void FftWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // 工具栏
    mainLayout->addWidget(createToolbar());

    // 图表区域
    setupChart();
    mainLayout->addWidget(m_chartView);

    setLayout(mainLayout);
}

QWidget* FftWidget::createToolbar()
{
    auto* toolbar = new QWidget(this);
    toolbar->setObjectName(QStringLiteral("FftToolbar"));
    auto* layout = new QHBoxLayout(toolbar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    // 通道选择
    auto* channelLabel = new QLabel(tr("通道:"), toolbar);
    channelLabel->setObjectName(QStringLiteral("FftChannelLabel"));
    m_channelCombo = new QComboBox(toolbar);
    m_channelCombo->setObjectName(QStringLiteral("FftChannelCombo"));
    m_channelCombo->setMinimumWidth(100);
    connect(m_channelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FftWidget::onChannelChanged);

    // 窗函数选择
    auto* windowLabel = new QLabel(tr("窗函数:"), toolbar);
    windowLabel->setObjectName(QStringLiteral("FftWindowLabel"));
    m_windowCombo = new QComboBox(toolbar);
    m_windowCombo->setObjectName(QStringLiteral("FftWindowCombo"));
    m_windowCombo->addItem(tr("矩形窗"), static_cast<int>(FftEngine::WindowType::Rectangular));
    m_windowCombo->addItem(tr("汉宁窗"), static_cast<int>(FftEngine::WindowType::Hanning));
    m_windowCombo->addItem(tr("海明窗"), static_cast<int>(FftEngine::WindowType::Hamming));
    m_windowCombo->addItem(tr("布莱克曼窗"), static_cast<int>(FftEngine::WindowType::Blackman));
    m_windowCombo->setCurrentIndex(1); // 默认汉宁窗
    connect(m_windowCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FftWidget::onWindowChanged);

    // FFT大小选择
    auto* fftSizeLabel = new QLabel(tr("FFT大小:"), toolbar);
    fftSizeLabel->setObjectName(QStringLiteral("FftSizeLabel"));
    m_fftSizeCombo = new QComboBox(toolbar);
    m_fftSizeCombo->setObjectName(QStringLiteral("FftSizeCombo"));
    populateFftSizes();
    m_fftSizeCombo->setCurrentIndex(2); // 默认1024
    connect(m_fftSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FftWidget::onFftSizeChanged);

    // 采样率输入
    auto* rateLabel = new QLabel(tr("采样率(Hz):"), toolbar);
    rateLabel->setObjectName(QStringLiteral("FftRateLabel"));
    m_sampleRateSpin = new QSpinBox(toolbar);
    m_sampleRateSpin->setObjectName(QStringLiteral("FftSampleRateSpin"));
    m_sampleRateSpin->setRange(1, 10000000);
    m_sampleRateSpin->setValue(static_cast<int>(m_sampleRate));
    m_sampleRateSpin->setSingleStep(100);
    connect(m_sampleRateSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
        m_sampleRate = static_cast<double>(val);
    });

    // 刷新按钮
    m_refreshBtn = new QPushButton(tr("刷新"), toolbar);
    m_refreshBtn->setObjectName(QStringLiteral("FftRefreshBtn"));
    connect(m_refreshBtn, &QPushButton::clicked,
            this, &FftWidget::refreshSpectrum);

    // 自动刷新
    m_autoRefreshCheck = new QCheckBox(tr("自动刷新"), toolbar);
    m_autoRefreshCheck->setObjectName(QStringLiteral("FftAutoRefreshCheck"));
    m_autoRefreshCheck->setChecked(m_autoRefresh);
    connect(m_autoRefreshCheck, &QCheckBox::toggled,
            this, &FftWidget::onAutoRefreshToggled);

    // 信息标签
    m_infoLabel = new QLabel(toolbar);
    m_infoLabel->setObjectName(QStringLiteral("FftInfoLabel"));
    m_infoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 布局
    layout->addWidget(channelLabel);
    layout->addWidget(m_channelCombo);
    layout->addWidget(windowLabel);
    layout->addWidget(m_windowCombo);
    layout->addWidget(fftSizeLabel);
    layout->addWidget(m_fftSizeCombo);
    layout->addWidget(rateLabel);
    layout->addWidget(m_sampleRateSpin);
    layout->addWidget(m_refreshBtn);
    layout->addWidget(m_autoRefreshCheck);
    layout->addStretch();
    layout->addWidget(m_infoLabel);

    return toolbar;
}

void FftWidget::setupChart()
{
    m_chart = new QChart();
    m_chart->setObjectName(QStringLiteral("FftChart"));
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(2, 2, 2, 2));

    // 频谱曲线
    m_spectrumSeries = new QLineSeries(m_chart);
    m_spectrumSeries->setObjectName(QStringLiteral("FftSpectrumSeries"));
    m_chart->addSeries(m_spectrumSeries);

    // X轴: 频率 (Hz)
    m_xAxis = new QValueAxis(m_chart);
    m_xAxis->setObjectName(QStringLiteral("FftXAxis"));
    m_xAxis->setTitleText(tr("频率 (Hz)"));
    m_xAxis->setLabelFormat(QStringLiteral("%g"));
    m_xAxis->setRange(0, 500);

    // Y轴: 幅度
    m_yAxis = new QValueAxis(m_chart);
    m_yAxis->setObjectName(QStringLiteral("FftYAxis"));
    m_yAxis->setTitleText(tr("幅度"));
    m_yAxis->setLabelFormat(QStringLiteral("%g"));
    m_yAxis->setRange(0, 1);

    m_chart->addAxis(m_xAxis, Qt::AlignBottom);
    m_chart->addAxis(m_yAxis, Qt::AlignLeft);
    m_spectrumSeries->attachAxis(m_xAxis);
    m_spectrumSeries->attachAxis(m_yAxis);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setObjectName(QStringLiteral("FftChartView"));
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

void FftWidget::populateFftSizes()
{
    m_fftSizeCombo->addItem(QStringLiteral("256"),   256);
    m_fftSizeCombo->addItem(QStringLiteral("512"),   512);
    m_fftSizeCombo->addItem(QStringLiteral("1024"),  1024);
    m_fftSizeCombo->addItem(QStringLiteral("2048"),  2048);
    m_fftSizeCombo->addItem(QStringLiteral("4096"),  4096);
}

// ============================================================
// 频谱计算与显示
// ============================================================

void FftWidget::refreshSpectrum()
{
    if (!m_model || !m_spectrumSeries) {
        return;
    }

    // 获取当前选中通道名
    QString channel = m_channelCombo->currentText();
    if (channel.isEmpty()) {
        m_infoLabel->setText(tr("无通道数据"));
        return;
    }

    // 获取通道时域数据
    QVector<QPointF> timeData = m_model->channelData(channel);
    if (timeData.isEmpty()) {
        m_spectrumSeries->replace({});
        m_infoLabel->setText(tr("无数据"));
        return;
    }

    // 获取当前配置
    int fftSize = m_fftSizeCombo->currentData().toInt();
    auto window = static_cast<FftEngine::WindowType>(
        m_windowCombo->currentData().toInt());

    // 执行FFT计算
    QVector<QPointF> spectrum = m_engine->compute(
        timeData, m_sampleRate, window, fftSize);

    if (spectrum.isEmpty()) {
        m_spectrumSeries->replace({});
        m_infoLabel->setText(tr("计算失败"));
        return;
    }

    // 更新频谱曲线
    m_spectrumSeries->replace(spectrum);

    // 自动调整坐标轴范围
    double maxFreq = spectrum.last().x();
    double maxMag = 0.0;
    m_fundamentalFreq = 0.0;
    for (const auto& pt : spectrum) {
        if (pt.y() > maxMag) {
            maxMag = pt.y();
            m_fundamentalFreq = pt.x();
        }
    }

    m_xAxis->setRange(0, maxFreq);
    m_yAxis->setRange(0, qMax(maxMag * 1.1, 0.001)); // 留10%余量

    // 更新信息标签
    m_infoLabel->setText(
        tr("基频: %1 Hz | 点数: %2")
            .arg(m_fundamentalFreq, 0, 'f', 1)
            .arg(timeData.size()));
}

// ============================================================
// 槽函数
// ============================================================

void FftWidget::onChannelChanged(int /*index*/)
{
    if (m_autoRefresh) {
        refreshSpectrum();
    }
}

void FftWidget::onWindowChanged(int /*index*/)
{
    if (m_autoRefresh) {
        refreshSpectrum();
    }
}

void FftWidget::onFftSizeChanged(int /*value*/)
{
    if (m_autoRefresh) {
        refreshSpectrum();
    }
}

void FftWidget::onAutoRefreshToggled(bool checked)
{
    m_autoRefresh = checked;
}

void FftWidget::onDataUpdated(const QStringList& updatedChannels)
{
    if (!m_autoRefresh) {
        return;
    }

    // 只在当前选中通道有新数据时才刷新
    QString currentChannel = m_channelCombo->currentText();
    if (updatedChannels.contains(currentChannel)) {
        refreshSpectrum();
    }
}

void FftWidget::onChannelsChanged()
{
    if (!m_model) {
        return;
    }

    // 保存当前选择
    QString prevChannel = m_channelCombo->currentText();

    // 重建通道列表
    m_channelCombo->blockSignals(true);
    m_channelCombo->clear();
    QStringList names = m_model->channelNames();
    for (const auto& name : names) {
        m_channelCombo->addItem(name);
    }

    // 恢复之前的选择（如果仍存在）
    int idx = m_channelCombo->findText(prevChannel);
    if (idx >= 0) {
        m_channelCombo->setCurrentIndex(idx);
    } else if (m_channelCombo->count() > 0) {
        m_channelCombo->setCurrentIndex(0);
    }
    m_channelCombo->blockSignals(false);

    // 自动刷新频谱
    if (m_autoRefresh && m_channelCombo->count() > 0) {
        refreshSpectrum();
    }
}

void FftWidget::onThemeChanged()
{
    applyThemeColors();
}

// ============================================================
// 主题样式
// ============================================================

void FftWidget::applyThemeColors()
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

    // 频谱线条颜色: 使用ChartColors调色板第一色（蓝色系）
    bool isDark = (theme.currentTheme().startsWith(QStringLiteral("dark")));
    const auto& colors = ChartColors::colorsForTheme(isDark);
    QColor spectrumColor = colors.isEmpty() ? QColor("#89b4fa") : colors.first();
    m_spectrumSeries->setPen(QPen(spectrumColor, 1.5));

    // 图表绘图区背景（与整体背景保持一致）
    QBrush plotAreaBrush(bgColor);
    m_chart->setPlotAreaBackgroundBrush(plotAreaBrush);
    m_chart->setPlotAreaBackgroundVisible(true);
}
