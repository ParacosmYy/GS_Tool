/**
 * @file PidTunerWidget.cpp
 * @brief PID调试器面板实现 — 布局、仿真触发、QPainter响应曲线
 *
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/pid/PidTunerWidget.h"
#include "utils/pid/PidSimulator.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <algorithm>

/* ============================================================
 * 构造
 * ============================================================ */

PidTunerWidget::PidTunerWidget(QWidget* parent)
    : QWidget(parent)
    , m_running(false)
{
    setObjectName(QStringLiteral("pidTunerWidget"));

    m_simulator = new PidSimulator(this);

    /* 主布局: 左参数面板 + 右曲线区域 */
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    mainLayout->addWidget(createParamPanel(), 0);
    mainLayout->addWidget(createChartArea(), 1);

    /* 连接信号 */
    connect(m_btnStart, &QPushButton::clicked,
            this, &PidTunerWidget::onStartClicked);
    connect(m_btnStop,  &QPushButton::clicked,
            this, &PidTunerWidget::onStopClicked);
    connect(m_btnStep,  &QPushButton::clicked,
            this, &PidTunerWidget::onStepClicked);

    /* 参数变化时自动重新仿真 */
    const auto spinners = {m_spinKp, m_spinKi, m_spinKd, m_spinSetpoint,
                           m_spinGain, m_spinTau, m_spinDelay, m_spinNoise};
    for (auto* spin : spinners) {
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &PidTunerWidget::onParamChanged);
    }
    connect(m_comboType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PidTunerWidget::onParamChanged);

    /* 初始仿真 */
    runSimulation();
}

/* ============================================================
 * 参数面板创建
 * ============================================================ */

QWidget* PidTunerWidget::createParamPanel()
{
    auto* panel = new QWidget(this);
    panel->setObjectName(QStringLiteral("pidParamPanel"));
    panel->setFixedWidth(240);

    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    /* ---- PID 参数组 ---- */
    auto* pidGroup = new QGroupBox(tr("PID Parameters"), panel);
    pidGroup->setObjectName(QStringLiteral("pidParamGroup"));
    auto* pidForm = new QFormLayout(pidGroup);

    auto makeSpin = [&](double min, double max, double step, double val) {
        auto* s = new QDoubleSpinBox(pidGroup);
        s->setRange(min, max);
        s->setSingleStep(step);
        s->setDecimals(3);
        s->setValue(val);
        return s;
    };

    m_spinKp       = makeSpin(-100, 100, 0.1, 1.0);
    m_spinKi       = makeSpin(-100, 100, 0.01, 0.0);
    m_spinKd       = makeSpin(-100, 100, 0.01, 0.0);
    m_spinSetpoint = makeSpin(-1000, 1000, 0.1, 1.0);

    m_spinKp->setObjectName(QStringLiteral("spinKp"));
    m_spinKi->setObjectName(QStringLiteral("spinKi"));
    m_spinKd->setObjectName(QStringLiteral("spinKd"));
    m_spinSetpoint->setObjectName(QStringLiteral("spinSetpoint"));

    pidForm->addRow(QStringLiteral("Kp:"), m_spinKp);
    pidForm->addRow(QStringLiteral("Ki:"), m_spinKi);
    pidForm->addRow(QStringLiteral("Kd:"), m_spinKd);
    pidForm->addRow(tr("Setpoint:"), m_spinSetpoint);
    layout->addWidget(pidGroup);

    /* ---- 被控对象参数组 ---- */
    auto* plantGroup = new QGroupBox(tr("Plant Model"), panel);
    plantGroup->setObjectName(QStringLiteral("plantParamGroup"));
    auto* plantForm = new QFormLayout(plantGroup);

    m_spinGain  = makeSpin(0.01, 100, 0.1, 1.0);
    m_spinTau   = makeSpin(0.001, 100, 0.1, 1.0);
    m_spinDelay = makeSpin(0.0, 10.0, 0.01, 0.0);
    m_spinNoise = makeSpin(0.0, 1.0, 0.001, 0.0);

    m_spinGain->setObjectName(QStringLiteral("spinGain"));
    m_spinTau->setObjectName(QStringLiteral("spinTau"));
    m_spinDelay->setObjectName(QStringLiteral("spinDelay"));
    m_spinNoise->setObjectName(QStringLiteral("spinNoise"));

    plantForm->addRow(tr("Gain K:"), m_spinGain);
    plantForm->addRow(tr("Time Const T:"), m_spinTau);
    plantForm->addRow(tr("Delay τ:"), m_spinDelay);
    plantForm->addRow(tr("Noise σ:"), m_spinNoise);
    layout->addWidget(plantGroup);

    /* ---- 激励类型 ---- */
    auto* typeGroup = new QGroupBox(tr("Excitation"), panel);
    typeGroup->setObjectName(QStringLiteral("excitationGroup"));
    auto* typeLayout = new QVBoxLayout(typeGroup);

    m_comboType = new QComboBox(typeGroup);
    m_comboType->setObjectName(QStringLiteral("comboResponseType"));
    m_comboType->addItem(tr("Step"),     static_cast<int>(PidResponseType::Step));
    m_comboType->addItem(tr("Ramp"),     static_cast<int>(PidResponseType::Ramp));
    m_comboType->addItem(tr("Sine"),     static_cast<int>(PidResponseType::Sine));
    m_comboType->addItem(tr("Square"),   static_cast<int>(PidResponseType::Square));
    m_comboType->addItem(tr("Impulse"),  static_cast<int>(PidResponseType::Impulse));
    typeLayout->addWidget(m_comboType);
    layout->addWidget(typeGroup);

    /* ---- 按钮行 ---- */
    auto* btnLayout = new QHBoxLayout();
    m_btnStart = new QPushButton(tr("Start"), panel);
    m_btnStop  = new QPushButton(tr("Stop"),  panel);
    m_btnStep  = new QPushButton(tr("Step"),  panel);

    m_btnStart->setObjectName(QStringLiteral("btnPidStart"));
    m_btnStop->setObjectName(QStringLiteral("btnPidStop"));
    m_btnStep->setObjectName(QStringLiteral("btnPidStep"));

    btnLayout->addWidget(m_btnStart);
    btnLayout->addWidget(m_btnStop);
    btnLayout->addWidget(m_btnStep);
    layout->addLayout(btnLayout);

    /* ---- 性能指标标签 ---- */
    auto* metricsGroup = new QGroupBox(tr("Performance"), panel);
    metricsGroup->setObjectName(QStringLiteral("metricsGroup"));
    auto* metricsLayout = new QFormLayout(metricsGroup);

    m_labelOvershoot = new QLabel(QStringLiteral("--"), metricsGroup);
    m_labelSettling  = new QLabel(QStringLiteral("--"), metricsGroup);
    m_labelRise      = new QLabel(QStringLiteral("--"), metricsGroup);
    m_labelSSE       = new QLabel(QStringLiteral("--"), metricsGroup);

    m_labelOvershoot->setObjectName(QStringLiteral("labelOvershoot"));
    m_labelSettling->setObjectName(QStringLiteral("labelSettling"));
    m_labelRise->setObjectName(QStringLiteral("labelRise"));
    m_labelSSE->setObjectName(QStringLiteral("labelSSE"));

    metricsLayout->addRow(tr("Overshoot %:"), m_labelOvershoot);
    metricsLayout->addRow(tr("Settling (s):"), m_labelSettling);
    metricsLayout->addRow(tr("Rise (s):"),     m_labelRise);
    metricsLayout->addRow(tr("SS Error:"),     m_labelSSE);
    layout->addWidget(metricsGroup);

    layout->addStretch();
    return panel;
}

/* ============================================================
 * 曲线区域创建
 * ============================================================ */

QWidget* PidTunerWidget::createChartArea()
{
    auto* area = new QWidget(this);
    area->setObjectName(QStringLiteral("pidChartArea"));
    area->setMinimumSize(400, 300);
    return area;
}

/* ============================================================
 * 按钮处理
 * ============================================================ */

void PidTunerWidget::onStartClicked()
{
    m_running = true;
    runSimulation();
}

void PidTunerWidget::onStopClicked()
{
    m_running = false;
}

void PidTunerWidget::onStepClicked()
{
    m_running = false;
    runSimulation();
}

void PidTunerWidget::onParamChanged()
{
    emit parametersChanged();
    if (m_running) {
        runSimulation();
    }
}

/* ============================================================
 * 仿真执行
 * ============================================================ */

void PidTunerWidget::runSimulation()
{
    /* 读取PID参数 */
    PidParams pid;
    pid.kp         = m_spinKp->value();
    pid.ki         = m_spinKi->value();
    pid.kd         = m_spinKd->value();
    pid.setpoint   = m_spinSetpoint->value();

    /* 读取被控对象参数 */
    PidSimulator::PlantParams plant;
    plant.gain         = m_spinGain->value();
    plant.timeConstant = m_spinTau->value();
    plant.delay        = m_spinDelay->value();
    plant.noiseStddev  = m_spinNoise->value();

    m_simulator->setPlantParams(plant);

    /* 激励类型 */
    const int typeIdx = m_comboType->currentData().toInt();
    const auto type   = static_cast<PidResponseType>(typeIdx);

    /* 执行仿真 */
    m_lastResponse = m_simulator->simulate(pid, type);

    /* 更新UI */
    updateMetricsLabels(m_lastResponse);
    update();

    ++m_stats.simulationsTriggered;
}

/* ============================================================
 * 性能指标更新
 * ============================================================ */

void PidTunerWidget::updateMetricsLabels(const PidResponse& resp)
{
    m_labelOvershoot->setText(QString::number(resp.overshoot, 'f', 2));
    m_labelSettling->setText(QString::number(resp.settlingTime, 'f', 3));
    m_labelRise->setText(QString::number(resp.riseTime, 'f', 3));
    m_labelSSE->setText(QString::number(resp.steadyStateError, 'f', 4));
}

/* ============================================================
 * paintEvent: 自绘响应曲线
 * ============================================================ */

void PidTunerWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    ++m_stats.repaintCount;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    /* 绘图区域 (右侧) */
    const int chartX = 250;
    const int chartW = width() - chartX - 10;
    const int chartH = height() - 20;

    if (chartW <= 0 || chartH <= 0) return;

    const QRectF chartRect(chartX, 10, chartW, chartH);

    /* 背景 */
    p.fillRect(chartRect, QColor(30, 30, 40));

    /* 网格线 */
    p.setPen(QPen(QColor(60, 60, 70), 1, Qt::DotLine));
    for (int i = 1; i < 5; ++i) {
        const double y = chartRect.top() + chartRect.height() * i / 5.0;
        p.drawLine(QPointF(chartRect.left(), y), QPointF(chartRect.right(), y));
    }
    for (int i = 1; i < 10; ++i) {
        const double x = chartRect.left() + chartRect.width() * i / 10.0;
        p.drawLine(QPointF(x, chartRect.top()), QPointF(x, chartRect.bottom()));
    }

    const auto& resp = m_lastResponse;
    if (resp.processData.isEmpty()) return;

    /* 数据范围 */
    const int n = resp.processData.size();
    double yMin = *std::min_element(resp.processData.constBegin(), resp.processData.constEnd());
    double yMax = *std::max_element(resp.processData.constBegin(), resp.processData.constEnd());

    for (int i = 0; i < n; ++i) {
        yMin = qMin(yMin, resp.setpointData[i]);
        yMax = qMax(yMax, resp.setpointData[i]);
    }
    const double yRange = qMax(yMax - yMin, 1e-6);
    const double yPad   = yRange * 0.1;
    yMin -= yPad;
    yMax += yPad;

    /* 坐标映射 */
    auto mapX = [&](int idx) -> double {
        return chartRect.left() + (static_cast<double>(idx) / (n - 1)) * chartRect.width();
    };
    auto mapY = [&](double val) -> double {
        return chartRect.bottom() - ((val - yMin) / (yMax - yMin)) * chartRect.height();
    };

    /* 设定值曲线 (橙色虚线) */
    QPen spPen(QColor(255, 165, 0), 1.5, Qt::DashLine);
    p.setPen(spPen);
    for (int i = 1; i < n; ++i) {
        p.drawLine(QPointF(mapX(i - 1), mapY(resp.setpointData[i - 1])),
                   QPointF(mapX(i),     mapY(resp.setpointData[i])));
    }

    /* 过程变量曲线 (绿色实线) */
    QPen pvPen(QColor(0, 200, 100), 2.0);
    p.setPen(pvPen);
    for (int i = 1; i < n; ++i) {
        p.drawLine(QPointF(mapX(i - 1), mapY(resp.processData[i - 1])),
                   QPointF(mapX(i),     mapY(resp.processData[i])));
    }

    /* 控制器输出曲线 (青色细线) */
    QPen ctrlPen(QColor(0, 180, 255), 1.0, Qt::DotLine);
    p.setPen(ctrlPen);
    for (int i = 1; i < n; ++i) {
        p.drawLine(QPointF(mapX(i - 1), mapY(resp.outputData[i - 1])),
                   QPointF(mapX(i),     mapY(resp.outputData[i])));
    }

    /* 图例 */
    p.setPen(QColor(200, 200, 200));
    p.setFont(QFont(QStringLiteral("Segoe UI"), 9));
    const qreal legendY = chartRect.top() + 14;
    const qreal legendX = chartRect.left() + 8;

    p.setPen(spPen);   p.drawText(legendX, legendY,        tr("Setpoint"));
    p.setPen(pvPen);   p.drawText(legendX, legendY + 16,   tr("Output"));
    p.setPen(ctrlPen); p.drawText(legendX, legendY + 32,   tr("Control"));
}
