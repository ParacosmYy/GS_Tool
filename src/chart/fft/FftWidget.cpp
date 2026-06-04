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

/** @brief 获取累计FFT变换次数 */
quint64 FftWidget::totalTransforms() const
{
    return m_totalTransforms;
}

/** @brief 获取累计峰值搜索次数(频谱最大幅度检测) */
quint64 FftWidget::totalPeakSearches() const
{
    return m_totalPeakSearches;
}

/** @brief 获取累计窗函数变更次数 */
quint64 FftWidget::totalWindowChanges() const
{
    return m_totalWindowChanges;
}

/** @brief 获取累计FFT大小变更次数 */
quint64 FftWidget::totalSizeChanges() const
{
    return m_totalSizeChanges;
}

/** @brief 重置所有FFT控件统计计数器 */
void FftWidget::resetFftWidgetStatistics()
{
    m_totalTransforms = 0;
    m_totalPeakSearches = 0;
    m_totalWindowChanges = 0;
    m_totalSizeChanges = 0;
}

// ============================================================
// 频谱计算与显示
// ============================================================

/** @brief 刷新频谱: 从ChartModel读取通道数据→FftEngine计算→更新曲线和坐标轴 */
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
    ++m_totalTransforms;

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
    ++m_totalPeakSearches;

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

/** @brief 通道选择变更槽函数，自动刷新模式下触发频谱重算 @param index 下拉框新索引(未使用) */
void FftWidget::onChannelChanged(int /*index*/)
{
    if (m_autoRefresh) {
        refreshSpectrum();
    }
}

/** @brief 窗函数选择变更槽函数，自动刷新模式下触发频谱重算 @param index 下拉框新索引(未使用) */
void FftWidget::onWindowChanged(int /*index*/)
{
    ++m_totalWindowChanges;
    if (m_autoRefresh) {
        refreshSpectrum();
    }
}

/** @brief FFT大小变更槽函数，自动刷新模式下触发频谱重算 @param value 新的FFT大小(未使用) */
void FftWidget::onFftSizeChanged(int /*value*/)
{
    ++m_totalSizeChanges;
    if (m_autoRefresh) {
        refreshSpectrum();
    }
}

/** @brief 自动刷新开关切换槽函数 @param checked true=开启自动刷新 */
void FftWidget::onAutoRefreshToggled(bool checked)
{
    m_autoRefresh = checked;
}

/** @brief ChartModel数据更新槽函数，仅当当前通道有新数据时自动刷新 @param updatedChannels 本次更新的通道名列表 */
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

/** @brief 通道列表变更槽函数，重建通道下拉框并尝试恢复之前的选择 */
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

/** @brief 主题切换槽函数，重新应用颜色到图表 */
void FftWidget::onThemeChanged()
{
    applyThemeColors();
}
