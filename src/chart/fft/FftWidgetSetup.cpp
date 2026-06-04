/**
 * @file FftWidgetSetup.cpp
 * @brief FFT频谱控件 -- UI搭建、工具栏创建与图表初始化
 *
 * 本文件从FftWidget.cpp拆分而来，集中管理FftWidget的视觉层构建逻辑:
 *   - setupUI():        主布局编排（工具栏 + 图表区域）
 *   - createToolbar():  顶部配置栏（通道/窗函数/FFT大小/采样率/刷新按钮）
 *   - setupChart():     QChart频谱图表创建（曲线、X/Y坐标轴、视图）
 *   - populateFftSizes(): FFT大小下拉框选项填充
 *
 * 主题样式应用见 FftWidgetTheme.cpp。
 * 槽函数与统计接口见 FftWidgetSlots.cpp。
 *
 * 所有方法均为FftWidget的private成员，声明见FftWidget.h。
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

// ============================================================
// UI布局搭建
// ============================================================

/** @brief 初始化UI布局(工具栏+图表区域) */
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

/** @brief 创建顶部配置工具栏(通道/窗函数/FFT大小/采样率/刷新按钮) @return 工具栏Widget指针 */
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

// ============================================================
// 图表初始化
// ============================================================

/** @brief 创建频谱图表区域(曲线+X/Y坐标轴+图表视图) */
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

/** @brief 填充FFT大小下拉框(256/512/1024/2048/4096) */
void FftWidget::populateFftSizes()
{
    m_fftSizeCombo->addItem(QStringLiteral("256"),   256);
    m_fftSizeCombo->addItem(QStringLiteral("512"),   512);
    m_fftSizeCombo->addItem(QStringLiteral("1024"),  1024);
    m_fftSizeCombo->addItem(QStringLiteral("2048"),  2048);
    m_fftSizeCombo->addItem(QStringLiteral("4096"),  4096);
}

// 主题样式应用见 FftWidgetTheme.cpp
