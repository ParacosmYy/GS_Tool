/**
 * @file FftWidget.cpp
 * @brief FFT频谱显示控件实现 -- 构造、频谱计算、槽函数与统计接口
 *
 * 本文件包含FftWidget的核心逻辑层:
 *   - 构造函数: 初始化引擎、信号连接、首次主题/通道加载
 *   - 公共接口: setSampleRate / sampleRate / 统计计数器
 *   - refreshSpectrum(): 从ChartModel读取时域数据→FftEngine计算→更新曲线和坐标轴
 *   - 槽函数: 通道切换、窗函数切换、FFT大小变更、自动刷新、数据更新、主题切换
 *
 * UI搭建和主题样式方法见 FftWidgetSetup.cpp。
 */

#include "chart/fft/FftWidget.h"
#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

#include <QComboBox>
#include <QtCharts>
#include <algorithm>

// ============================================================
// 构造 / 初始化
// ============================================================

/** @brief 构造FFT频谱控件 @param model 数据模型指针(外部拥有) @param parent 父控件 */
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

/** @brief 设置采样率(Hz)，用于计算频率轴 @param rate 采样率(Hz)，无效值自动修正为1000Hz */
void FftWidget::setSampleRate(double rate)
{
    m_sampleRate = (rate > 0.0) ? rate : 1000.0;
    if (m_sampleRateSpin) {
        m_sampleRateSpin->setValue(static_cast<int>(m_sampleRate));
    }
}

/** @brief 获取当前采样率 @return 采样率(Hz) */
double FftWidget::sampleRate() const
{
    return m_sampleRate;
}

// ---- 槽函数/统计计数器见 FftWidgetSlots.cpp ----

// ============================================================
// 频谱计算
// ============================================================

/** @brief 从ChartModel读取时域数据→FftEngine计算→更新曲线和坐标轴 */
void FftWidget::refreshSpectrum()
{
    if (!m_model || !m_spectrumSeries) {
        return;
    }

    QString channel = m_channelCombo->currentText();
    if (channel.isEmpty()) {
        m_infoLabel->setText(tr("无通道数据"));
        ++m_totalRenderErrors;
        return;
    }

    QVector<QPointF> timeData = m_model->channelData(channel);
    if (timeData.isEmpty()) {
        m_spectrumSeries->replace({});
        m_infoLabel->setText(tr("无数据"));
        ++m_totalRenderErrors;
        return;
    }

    int fftSize = m_fftSizeCombo->currentData().toInt();
    auto window = static_cast<FftEngine::WindowType>(
        m_windowCombo->currentData().toInt());

    QVector<QPointF> spectrum = m_engine->compute(
        timeData, m_sampleRate, window, fftSize);

    if (spectrum.isEmpty()) {
        m_spectrumSeries->replace({});
        m_infoLabel->setText(tr("计算失败"));
        ++m_totalRenderErrors;
        return;
    }

    m_spectrumSeries->replace(spectrum);
    ++m_totalTransforms;

    double maxFreq = spectrum.last().x();
    double maxMag = 0.0;
    m_fundamentalFreq = 0.0;
    for (const auto& pt : spectrum) {
        if (pt.y() > maxMag) {
            maxMag = pt.y();
            m_fundamentalFreq = pt.x();
        }
    }
    ++m_totalPeakSearches;

    m_xAxis->setRange(0, maxFreq);
    m_yAxis->setRange(0, qMax(maxMag * 1.1, 0.001));

    m_infoLabel->setText(
        tr("基频: %1 Hz | 点数: %2")
            .arg(m_fundamentalFreq, 0, 'f', 1)
            .arg(timeData.size()));
}
