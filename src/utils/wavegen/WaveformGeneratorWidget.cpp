/**
 * @file WaveformGeneratorWidget.cpp
 * @brief 波形发生器面板实现 -- 左侧参数控制 + 右侧 QPainter 波形预览
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/wavegen/WaveformGeneratorWidget.h"
#include "utils/wavegen/WaveformGenerator.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QtMath>

// ══════════════════════════════════════════════
// 构造
// ══════════════════════════════════════════════

WaveformGeneratorWidget::WaveformGeneratorWidget(QWidget *parent)
    : QWidget(parent)
    , m_generator(new WaveformGenerator(this))
{
    setupUI();
    updateParamsFromControls();
}

// ══════════════════════════════════════════════
// 公开接口
// ══════════════════════════════════════════════

WaveformGenerator *WaveformGeneratorWidget::generator() const
{
    return m_generator;
}

// stats() and resetStatistics() are in WaveformGeneratorWidgetStats.cpp

// ══════════════════════════════════════════════
// paintEvent -- 绘制波形预览
// ══════════════════════════════════════════════

void WaveformGeneratorWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    drawWaveform(p);
}

// ══════════════════════════════════════════════
// 槽函数
// ══════════════════════════════════════════════

void WaveformGeneratorWidget::onTypeChanged(int index)
{
    Q_UNUSED(index)
    bool isSquare = (m_typeCombo->currentData().toInt()
                     == static_cast<int>(WaveGen::WaveformType::Square));
    m_dutySlider->setEnabled(isSquare);
    m_dutyLabel->setEnabled(isSquare);
    updateParamsFromControls();
}

void WaveformGeneratorWidget::onFrequencyChanged(double v)
{
    Q_UNUSED(v)
    updateParamsFromControls();
}

void WaveformGeneratorWidget::onAmplitudeChanged(int v)
{
    double amp = v / 100.0;
    m_ampLabel->setText(QString::number(amp, 'f', 2));
    updateParamsFromControls();
}

void WaveformGeneratorWidget::onOffsetChanged(int v)
{
    double off = v / 100.0;
    m_offLabel->setText(QString::number(off, 'f', 2));
    updateParamsFromControls();
}

void WaveformGeneratorWidget::onPhaseChanged(int v)
{
    m_phaseLabel->setText(QString::fromUtf8("%1°").arg(v));
    updateParamsFromControls();
}

void WaveformGeneratorWidget::onDutyCycleChanged(int v)
{
    m_dutyLabel->setText(QString::fromUtf8("%1%").arg(v));
    updateParamsFromControls();
}

void WaveformGeneratorWidget::onStartClicked()
{
    updateParamsFromControls();
    m_generator->startStreaming(10);
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
}

void WaveformGeneratorWidget::onStopClicked()
{
    m_generator->stopStreaming();
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
}

// ══════════════════════════════════════════════
// setupUI
// ══════════════════════════════════════════════

void WaveformGeneratorWidget::setupUI()
{
    setObjectName(QStringLiteral("WaveformGeneratorWidget"));
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    auto *leftPanel = new QWidget(this);
    leftPanel->setObjectName(QStringLiteral("wavegenLeftPanel"));
    leftPanel->setFixedWidth(240);
    auto *form = new QFormLayout(leftPanel);
    form->setContentsMargins(4, 4, 4, 4);
    form->setSpacing(6);

    createControls(leftPanel, form);
    mainLayout->addWidget(leftPanel);

    auto *previewFrame = new QFrame(this);
    previewFrame->setObjectName(QStringLiteral("wavegenPreviewFrame"));
    previewFrame->setFrameShape(QFrame::StyledPanel);
    mainLayout->addWidget(previewFrame, 1);

    connectSignals();
}

void WaveformGeneratorWidget::createControls(QWidget *parent, QFormLayout *form)
{
    m_typeCombo = new QComboBox(parent);
    m_typeCombo->setObjectName(QStringLiteral("wavegenTypeCombo"));
    m_typeCombo->addItem(tr("Sine"),   static_cast<int>(WaveGen::WaveformType::Sine));
    m_typeCombo->addItem(tr("Square"), static_cast<int>(WaveGen::WaveformType::Square));
    m_typeCombo->addItem(tr("Triangle"), static_cast<int>(WaveGen::WaveformType::Triangle));
    m_typeCombo->addItem(tr("Sawtooth"), static_cast<int>(WaveGen::WaveformType::Sawtooth));
    m_typeCombo->addItem(tr("Noise"),  static_cast<int>(WaveGen::WaveformType::Noise));
    m_typeCombo->addItem(tr("Custom"), static_cast<int>(WaveGen::WaveformType::Custom));
    form->addRow(tr("Type:"), m_typeCombo);

    m_freqSpin = new QDoubleSpinBox(parent);
    m_freqSpin->setObjectName(QStringLiteral("wavegenFreqSpin"));
    m_freqSpin->setRange(0.01, 100000.0);
    m_freqSpin->setValue(1000.0);
    m_freqSpin->setSuffix(tr(" Hz"));
    m_freqSpin->setDecimals(1);
    form->addRow(tr("Frequency:"), m_freqSpin);

    m_ampLabel = new QLabel(QStringLiteral("1.00"), parent);
    m_ampLabel->setObjectName(QStringLiteral("wavegenAmpLabel"));
    m_ampSlider = new QSlider(Qt::Horizontal, parent);
    m_ampSlider->setObjectName(QStringLiteral("wavegenAmpSlider"));
    m_ampSlider->setRange(0, 200);
    m_ampSlider->setValue(100);
    form->addRow(tr("Amplitude:"), m_ampSlider);
    form->addRow(QString(), m_ampLabel);

    m_offLabel = new QLabel(QStringLiteral("0.00"), parent);
    m_offLabel->setObjectName(QStringLiteral("wavegenOffLabel"));
    m_offSlider = new QSlider(Qt::Horizontal, parent);
    m_offSlider->setObjectName(QStringLiteral("wavegenOffSlider"));
    m_offSlider->setRange(-100, 100);
    m_offSlider->setValue(0);
    form->addRow(tr("Offset:"), m_offSlider);
    form->addRow(QString(), m_offLabel);

    m_phaseLabel = new QLabel(QStringLiteral("0°"), parent);
    m_phaseLabel->setObjectName(QStringLiteral("wavegenPhaseLabel"));
    m_phaseSlider = new QSlider(Qt::Horizontal, parent);
    m_phaseSlider->setObjectName(QStringLiteral("wavegenPhaseSlider"));
    m_phaseSlider->setRange(0, 360);
    m_phaseSlider->setValue(0);
    form->addRow(tr("Phase:"), m_phaseSlider);
    form->addRow(QString(), m_phaseLabel);

    m_dutyLabel = new QLabel(QStringLiteral("50%"), parent);
    m_dutyLabel->setObjectName(QStringLiteral("wavegenDutyLabel"));
    m_dutySlider = new QSlider(Qt::Horizontal, parent);
    m_dutySlider->setObjectName(QStringLiteral("wavegenDutySlider"));
    m_dutySlider->setRange(1, 99);
    m_dutySlider->setValue(50);
    m_dutySlider->setEnabled(false);
    m_dutyLabel->setEnabled(false);
    form->addRow(tr("Duty:"), m_dutySlider);
    form->addRow(QString(), m_dutyLabel);

    auto *btnLayout = new QHBoxLayout();
    m_startBtn = new QPushButton(tr("Start"), parent);
    m_startBtn->setObjectName(QStringLiteral("wavegenStartBtn"));
    m_stopBtn = new QPushButton(tr("Stop"), parent);
    m_stopBtn->setObjectName(QStringLiteral("wavegenStopBtn"));
    m_stopBtn->setEnabled(false);
    btnLayout->addWidget(m_startBtn);
    btnLayout->addWidget(m_stopBtn);
    form->addRow(btnLayout);
}

void WaveformGeneratorWidget::connectSignals()
{
    connect(m_typeCombo, &QComboBox::currentIndexChanged,
            this, &WaveformGeneratorWidget::onTypeChanged);
    connect(m_freqSpin, &QDoubleSpinBox::valueChanged,
            this, &WaveformGeneratorWidget::onFrequencyChanged);
    connect(m_ampSlider, &QSlider::valueChanged,
            this, &WaveformGeneratorWidget::onAmplitudeChanged);
    connect(m_offSlider, &QSlider::valueChanged,
            this, &WaveformGeneratorWidget::onOffsetChanged);
    connect(m_phaseSlider, &QSlider::valueChanged,
            this, &WaveformGeneratorWidget::onPhaseChanged);
    connect(m_dutySlider, &QSlider::valueChanged,
            this, &WaveformGeneratorWidget::onDutyCycleChanged);
    connect(m_startBtn, &QPushButton::clicked,
            this, &WaveformGeneratorWidget::onStartClicked);
    connect(m_stopBtn, &QPushButton::clicked,
            this, &WaveformGeneratorWidget::onStopClicked);

    connect(m_generator, &WaveformGenerator::dataGenerated,
            this, &WaveformGeneratorWidget::dataGenerated);

    connect(m_generator, &WaveformGenerator::paramsChanged,
            this, QOverload<>::of(&QWidget::update));
}

// ══════════════════════════════════════════════
// updateParamsFromControls
// ══════════════════════════════════════════════

void WaveformGeneratorWidget::updateParamsFromControls()
{
    WaveGen::WaveGenParams p;
    p.type      = static_cast<WaveGen::WaveformType>(
                      m_typeCombo->currentData().toInt());
    p.frequency = m_freqSpin->value();
    p.amplitude = m_ampSlider->value() / 100.0;
    p.offset    = m_offSlider->value() / 100.0;
    p.phase     = qDegreesToRadians(static_cast<double>(m_phaseSlider->value()));
    p.dutyCycle = m_dutySlider->value() / 100.0;
    p.sampleRate = 48000;
    p.sampleCount = 4800;
    m_generator->setParams(p);
}

// ══════════════════════════════════════════════
// drawWaveform -- QPainter 绘制 2 个周期
// ══════════════════════════════════════════════

void WaveformGeneratorWidget::drawWaveform(QPainter &p)
{
    QRect r = rect();
    int leftW = 240 + 16;
    QRect canvas(r.x() + leftW + 4, r.y() + 4,
                 r.width() - leftW - 12, r.height() - 8);

    if (canvas.width() < 20 || canvas.height() < 20) {
        return;
    }

    // 背景
    p.fillRect(canvas, QColor(30, 30, 40));

    // 网格线
    p.setPen(QPen(QColor(60, 60, 80), 1, Qt::DotLine));
    int hMid = canvas.y() + canvas.height() / 2;
    p.drawLine(canvas.x(), hMid, canvas.right(), hMid);
    for (int i = 1; i < 4; ++i) {
        int xg = canvas.x() + canvas.width() * i / 4;
        p.drawLine(xg, canvas.y(), xg, canvas.bottom());
    }

    // 生成预览数据 (2 个周期)
    WaveGen::WaveGenParams previewParams = m_generator->params();
    int ptsPerCycle = qBound(50, canvas.width(), 500);
    int totalPts = ptsPerCycle * 2;
    previewParams.sampleRate = previewParams.frequency * ptsPerCycle;
    previewParams.sampleCount = totalPts;

    WaveformGenerator tempGen;
    tempGen.setParams(previewParams);
    QVector<double> samples = tempGen.generateRaw(totalPts);

    if (samples.isEmpty()) {
        return;
    }

    // 找最大最小值用于缩放
    double maxAbs = 0.001;
    for (double s : samples) {
        maxAbs = qMax(maxAbs, qAbs(s));
    }

    // 绘制波形路径
    QPainterPath path;
    double xStep = static_cast<double>(canvas.width()) / (totalPts - 1);
    double yCenter = canvas.y() + canvas.height() / 2.0;
    double yScale = (canvas.height() / 2.0 - 4) / maxAbs;

    for (int i = 0; i < totalPts; ++i) {
        double x = canvas.x() + i * xStep;
        double y = yCenter - samples[i] * yScale;
        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    p.setPen(QPen(QColor(0, 200, 255), 2));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);

    // 零线标注
    p.setPen(QPen(QColor(120, 120, 140), 1, Qt::DashLine));
    p.drawLine(canvas.x(), hMid, canvas.right(), hMid);

    // 边框
    p.setPen(QPen(QColor(80, 80, 100), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(canvas);
}
