/**
 * @file SignalGeneratorWidget.cpp
 * @brief 信号发生器面板实现 — 构造函数、UI布局、波形生成、格式转换、自动发送
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 统计查询与重置方法见：@see SignalGeneratorWidgetStats.cpp
 */

#include "utils/signal_gen/SignalGeneratorWidget.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QRandomGenerator>
#include <cmath>

// ──────────────────────────────────────────────
// 构造与 UI
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化面板
 */
SignalGeneratorWidget::SignalGeneratorWidget(QWidget *parent)
    : QWidget(parent)
    , m_autoSending(false)
    , m_autoTimer(new QTimer(this))
{
    setupUI();

    connect(m_genButton, &QPushButton::clicked,
            this, &SignalGeneratorWidget::onGenerate);
    connect(m_outputButton, &QPushButton::clicked,
            this, &SignalGeneratorWidget::onOutput);
    connect(m_autoButton, &QPushButton::clicked, this, [this]() {
        m_autoSending = !m_autoSending;
        if (m_autoSending) {
            m_autoButton->setText(tr("停止自动"));
            m_autoTimer->start(m_intervalSpin->value());
        } else {
            m_autoButton->setText(tr("自动发送"));
            m_autoTimer->stop();
        }
    });
    connect(m_autoTimer, &QTimer::timeout,
            this, &SignalGeneratorWidget::onAutoSendTick);
}

/**
 * @brief 初始化界面布局，所有控件设置 objectName 供 QSS 匹配
 */
void SignalGeneratorWidget::setupUI()
{
    setObjectName(QStringLiteral("SignalGeneratorWidget"));

    auto *form = new QFormLayout(this);

    // 波形类型
    m_typeCombo = new QComboBox(this);
    m_typeCombo->setObjectName("sgTypeCombo");
    m_typeCombo->addItem(tr("正弦波"), static_cast<int>(Sine));
    m_typeCombo->addItem(tr("方波"),   static_cast<int>(Square));
    m_typeCombo->addItem(tr("三角波"), static_cast<int>(Triangle));
    m_typeCombo->addItem(tr("锯齿波"), static_cast<int>(Sawtooth));
    m_typeCombo->addItem(tr("噪声"),   static_cast<int>(Noise));
    form->addRow(tr("波形类型："), m_typeCombo);

    // 频率
    m_freqSpin = new QDoubleSpinBox(this);
    m_freqSpin->setObjectName("sgFreqSpin");
    m_freqSpin->setRange(0.01, 10000.0);
    m_freqSpin->setValue(1.0);
    m_freqSpin->setDecimals(2);
    m_freqSpin->setSuffix(tr(" Hz"));
    form->addRow(tr("频率："), m_freqSpin);

    // 振幅
    m_ampSpin = new QDoubleSpinBox(this);
    m_ampSpin->setObjectName("sgAmpSpin");
    m_ampSpin->setRange(0.0, 65535.0);
    m_ampSpin->setValue(1.0);
    m_ampSpin->setDecimals(3);
    form->addRow(tr("振幅："), m_ampSpin);

    // 直流偏移
    m_offsetSpin = new QDoubleSpinBox(this);
    m_offsetSpin->setObjectName("sgOffsetSpin");
    m_offsetSpin->setRange(-65535.0, 65535.0);
    m_offsetSpin->setValue(0.0);
    m_offsetSpin->setDecimals(3);
    form->addRow(tr("直流偏移："), m_offsetSpin);

    // 采样点数
    m_samplesSpin = new QSpinBox(this);
    m_samplesSpin->setObjectName("sgSamplesSpin");
    m_samplesSpin->setRange(1, 100000);
    m_samplesSpin->setValue(256);
    form->addRow(tr("采样点数："), m_samplesSpin);

    // 输出格式
    m_formatCombo = new QComboBox(this);
    m_formatCombo->setObjectName("sgFormatCombo");
    m_formatCombo->addItem(tr("uint8"),  static_cast<int>(Uint8));
    m_formatCombo->addItem(tr("int16"),  static_cast<int>(Int16));
    m_formatCombo->addItem(tr("uint16"), static_cast<int>(Uint16));
    m_formatCombo->addItem(tr("float32"),static_cast<int>(Float32));
    form->addRow(tr("输出格式："), m_formatCombo);

    // 按钮行
    auto *btnLayout = new QHBoxLayout();
    m_genButton = new QPushButton(tr("生成"), this);
    m_genButton->setObjectName("sgGenButton");
    m_outputButton = new QPushButton(tr("输出字节"), this);
    m_outputButton->setObjectName("sgOutputButton");
    m_autoButton = new QPushButton(tr("自动发送"), this);
    m_autoButton->setObjectName("sgAutoButton");
    btnLayout->addWidget(m_genButton);
    btnLayout->addWidget(m_outputButton);
    btnLayout->addWidget(m_autoButton);
    form->addRow(btnLayout);

    // 自动发送间隔
    m_intervalSpin = new QSpinBox(this);
    m_intervalSpin->setObjectName("sgIntervalSpin");
    m_intervalSpin->setRange(10, 60000);
    m_intervalSpin->setValue(100);
    m_intervalSpin->setSuffix(tr(" ms"));
    form->addRow(tr("自动间隔："), m_intervalSpin);

    // 预览标签
    m_previewLabel = new QLabel(tr("尚未生成"), this);
    m_previewLabel->setObjectName("sgPreviewLabel");
    m_previewLabel->setWordWrap(true);
    form->addRow(tr("预览："), m_previewLabel);

    // 电平条
    m_levelBar = new QProgressBar(this);
    m_levelBar->setObjectName("sgLevelBar");
    m_levelBar->setRange(0, 100);
    m_levelBar->setValue(0);
    m_levelBar->setFormat(tr("电平: %p%"));
    form->addRow(tr("信号电平："), m_levelBar);
}

// ──────────────────────────────────────────────
// 波形生成
// ──────────────────────────────────────────────

/**
 * @brief 生成指定参数的波形采样数据
 * @param type 波形类型
 * @param freqHz 归一化频率(周期数)
 * @param amp 振幅
 * @param offset 直流偏移
 * @param samples 采样点数
 * @return 采样值向量
 */
QVector<double> SignalGeneratorWidget::generateSignal(
    SignalType type, double freqHz, double amp, double offset, int samples)
{
    QVector<double> out;
    out.reserve(samples);

    for (int i = 0; i < samples; ++i) {
        const double phase = (samples > 1)
            ? 2.0 * M_PI * freqHz * static_cast<double>(i) / static_cast<double>(samples)
            : 0.0;
        double val = 0.0;

        switch (type) {
        case Sine:
            val = amp * qSin(phase) + offset;
            break;
        case Square:
            val = amp * (qSin(phase) >= 0.0 ? 1.0 : -1.0) + offset;
            break;
        case Triangle:
            val = amp * (2.0 / M_PI) * qAsin(qSin(phase)) + offset;
            break;
        case Sawtooth:
            val = amp * (2.0 * std::fmod(phase / (2.0 * M_PI) + 1.0, 2.0) - 1.0) + offset;
            break;
        case Noise:
            val = amp * (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) + offset;
            break;
        }
        out.append(val);
    }

    // 统计更新
    ++m_stats.totalGenerations;
    m_stats.totalSamplesGenerated += static_cast<quint64>(samples);
    if (static_cast<quint64>(samples) > m_stats.peakSamplesPerGen) {
        m_stats.peakSamplesPerGen = static_cast<quint64>(samples);
    }
    return out;
}

/**
 * @brief 将采样值按格式转换为字节流
 * @param signal 采样数据
 * @param fmt 输出格式
 * @return 字节数组（小端序）
 */
QByteArray SignalGeneratorWidget::convertToBytes(
    const QVector<double> &signal, OutputFormat fmt) const
{
    QByteArray bytes;
    bytes.reserve(signal.size() * 4); // 最坏情况 float32

    for (const double v : signal) {
        switch (fmt) {
        case Uint8: {
            const quint8 u8 = static_cast<quint8>(qBound(0.0, v, 255.0));
            bytes.append(reinterpret_cast<const char *>(&u8), sizeof(u8));
            break;
        }
        case Int16: {
            const qint16 i16 = static_cast<qint16>(qBound(-32768.0, v, 32767.0));
            bytes.append(reinterpret_cast<const char *>(&i16), sizeof(i16));
            break;
        }
        case Uint16: {
            const quint16 u16 = static_cast<quint16>(qBound(0.0, v, 65535.0));
            bytes.append(reinterpret_cast<const char *>(&u16), sizeof(u16));
            break;
        }
        case Float32: {
            const float f32 = static_cast<float>(v);
            bytes.append(reinterpret_cast<const char *>(&f32), sizeof(f32));
            break;
        }
        }
    }
    return bytes;
}

// ──────────────────────────────────────────────
// 槽函数
// ──────────────────────────────────────────────

/** @brief 生成按钮点击：读取 UI 参数并生成波形 */
void SignalGeneratorWidget::onGenerate()
{
    const auto type = static_cast<SignalType>(m_typeCombo->currentData().toInt());
    m_lastSignal = generateSignal(type, m_freqSpin->value(),
                                  m_ampSpin->value(), m_offsetSpin->value(),
                                  m_samplesSpin->value());
    updatePreview();
    emit signalGenerated(m_lastSignal);
}

/** @brief 输出按钮点击：将最近生成的波形转换为字节流并发出 */
void SignalGeneratorWidget::onOutput()
{
    if (m_lastSignal.isEmpty()) {
        onGenerate(); // 未生成过则先生成一次
    }
    if (m_lastSignal.isEmpty()) {
        ++m_stats.errorCount;
        return;
    }

    const auto fmt = static_cast<OutputFormat>(m_formatCombo->currentData().toInt());
    m_lastBytes = convertToBytes(m_lastSignal, fmt);

    const qint64 byteCount = m_lastBytes.size();
    m_stats.totalBytesOutput += static_cast<quint64>(byteCount);
    ++m_stats.totalOutputs;

    emit dataReady(m_lastBytes);
    emit outputComplete(byteCount);
}

/** @brief 自动发送定时器回调：周期性生成并输出 */
void SignalGeneratorWidget::onAutoSendTick()
{
    onGenerate();
    onOutput();
}

// ──────────────────────────────────────────────
// 预览
// ──────────────────────────────────────────────

/** @brief 刷新预览标签（前10个值 + 最小/最大/均值）和电平条 */
void SignalGeneratorWidget::updatePreview()
{
    if (m_lastSignal.isEmpty()) {
        m_previewLabel->setText(tr("尚未生成"));
        m_levelBar->setValue(0);
        return;
    }

    // 前10个值
    QStringList preview;
    const int show = qMin(10, m_lastSignal.size());
    for (int i = 0; i < show; ++i) {
        preview << QString::number(m_lastSignal[i], 'f', 3);
    }
    const QString more = (m_lastSignal.size() > 10)
        ? QStringLiteral(" ...") : QString();

    // 统计: 最小/最大/均值
    double minV = m_lastSignal[0], maxV = m_lastSignal[0], sum = 0.0;
    for (const double v : m_lastSignal) {
        if (v < minV) minV = v;
        if (v > maxV) maxV = v;
        sum += v;
    }
    const double mean = sum / static_cast<double>(m_lastSignal.size());

    m_previewLabel->setText(tr("[%1]%2\n最小: %3  最大: %4  均值: %5")
        .arg(preview.join(", "), more,
             QString::number(minV, 'f', 3),
             QString::number(maxV, 'f', 3),
             QString::number(mean, 'f', 3)));

    // 电平条: 用最大绝对值映射到 0~100
    const double peak = qMax(qAbs(minV), qAbs(maxV));
    const int level = qBound(0, static_cast<int>(peak / qMax(m_ampSpin->value(), 1e-9) * 50.0), 100);
    m_levelBar->setValue(level);
}
