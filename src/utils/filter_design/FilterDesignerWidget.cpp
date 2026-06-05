/**
 * @file FilterDesignerWidget.cpp
 * @brief 数字滤波器可视化设计控件实现 — 参数面板 + 频率响应QPainter绘制
 *
 * 布局: QSplitter(水平) → 左侧参数面板 + 右侧绘图区
 * 交互: 控件变更 → redesignFilter() → update() → paintEvent()
 */

#include "utils/filter_design/FilterDesignerWidget.h"
#include "utils/filter_design/FilterDesigner.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSplitter>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QtMath>

// ============================================================
// 构造
// ============================================================

/** @brief 构造滤波器设计控件 @param parent 父控件 */
FilterDesignerWidget::FilterDesignerWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("filterDesignerWidget"));

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName(QStringLiteral("filterDesignerSplitter"));

    QWidget* paramPanel = createParameterPanel();
    QWidget* plotArea   = createPlotArea();

    splitter->addWidget(paramPanel);
    splitter->addWidget(plotArea);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({240, 600});

    mainLayout->addWidget(splitter);

    setMinimumSize(640, 400);
}

// ============================================================
// 外部接口
// ============================================================

/** @brief 绑定滤波器设计引擎 @param designer 引擎指针 */
void FilterDesignerWidget::setDesigner(FilterDesigner* designer)
{
    m_designer = designer;
    redesignFilter();
}

/** @brief 获取当前滤波器参数 @return 参数集 */
FilterParams FilterDesignerWidget::currentParams() const
{
    return m_params;
}

/** @brief 获取当前滤波器系数 @return 系数 */
FilterCoeffs FilterDesignerWidget::currentCoeffs() const
{
    return m_coeffs;
}

/** @brief 获取当前频率响应 @return 响应数据 */
FilterResponse FilterDesignerWidget::currentResponse() const
{
    return m_response;
}

// ============================================================
// paintEvent
// ============================================================

/** @brief 自定义绘制 — 网格 + 幅度曲线 + 可选相位 */
void FilterDesignerWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    ++m_totalRepaints;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QMargins margins(60, 30, 20, 40);
    const QRectF plotRect = QRectF(rect()).marginsRemoved(margins);

    if (plotRect.width() < 10 || plotRect.height() < 10) {
        return;
    }

    painter.fillRect(rect(), QColor(30, 30, 30));

    drawGrid(painter, plotRect);
    drawMagnitudeCurve(painter, plotRect);

    if (m_phaseCheck && m_phaseCheck->isChecked()) {
        drawPhaseCurve(painter, plotRect);
    }

    drawAxisLabels(painter, plotRect);
}

// ============================================================
// 参数变更
// ============================================================

/** @brief 参数控件值变更时触发重新设计 */
void FilterDesignerWidget::onParameterChanged()
{
    ++m_totalParamChanges;

    m_params.type       = static_cast<FilterType>(m_typeCombo->currentData().toInt());
    m_params.family     = static_cast<FilterFamily>(m_familyCombo->currentData().toInt());
    m_params.order      = m_orderSpin->value();
    m_params.cutoffFreq = m_cutoffSpin->value();
    m_params.sampleRate = m_sampleSpin->value();
    m_params.window     = static_cast<WindowType>(m_windowCombo->currentData().toInt());

    bool isFirVisible = (m_params.family == FilterFamily::FIR);
    m_windowCombo->setVisible(isFirVisible);

    redesignFilter();
}

// ============================================================
// 重新设计
// ============================================================

/** @brief 根据当前参数重新设计滤波器并计算频响 */
void FilterDesignerWidget::redesignFilter()
{
    if (!m_designer) {
        return;
    }

    if (m_params.family == FilterFamily::FIR) {
        m_coeffs = m_designer->designFIR(m_params);
    } else {
        m_coeffs = m_designer->designIIR(m_params);
    }

    m_response = m_designer->frequencyResponse(m_coeffs, m_params.sampleRate, 512);
    update();

    emit filterChanged();
}

// ============================================================
// UI 构建 — 参数面板
// ============================================================

/** @brief 创建左侧参数面板 @return 面板Widget */
QWidget* FilterDesignerWidget::createParameterPanel()
{
    auto* panel = new QWidget(this);
    panel->setObjectName(QStringLiteral("filterParamPanel"));
    panel->setFixedWidth(240);

    auto* layout = new QFormLayout(panel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    // 滤波器类型
    m_typeCombo = new QComboBox(panel);
    m_typeCombo->setObjectName(QStringLiteral("filterTypeCombo"));
    m_typeCombo->addItem(tr("Low Pass"),   static_cast<int>(FilterType::LowPass));
    m_typeCombo->addItem(tr("High Pass"),  static_cast<int>(FilterType::HighPass));
    m_typeCombo->addItem(tr("Band Pass"),  static_cast<int>(FilterType::BandPass));
    m_typeCombo->addItem(tr("Band Stop"),  static_cast<int>(FilterType::BandStop));
    layout->addRow(tr("Type:"), m_typeCombo);

    // 滤波器族
    m_familyCombo = new QComboBox(panel);
    m_familyCombo->setObjectName(QStringLiteral("filterFamilyCombo"));
    m_familyCombo->addItem(tr("Butterworth"),   static_cast<int>(FilterFamily::Butterworth));
    m_familyCombo->addItem(tr("Chebyshev I"),   static_cast<int>(FilterFamily::Chebyshev1));
    m_familyCombo->addItem(tr("Chebyshev II"),  static_cast<int>(FilterFamily::Chebyshev2));
    m_familyCombo->addItem(tr("Bessel"),        static_cast<int>(FilterFamily::Bessel));
    m_familyCombo->addItem(tr("FIR"),           static_cast<int>(FilterFamily::FIR));
    layout->addRow(tr("Family:"), m_familyCombo);

    // 阶数
    m_orderSpin = new QSpinBox(panel);
    m_orderSpin->setObjectName(QStringLiteral("filterOrderSpin"));
    m_orderSpin->setRange(1, 20);
    m_orderSpin->setValue(m_params.order);
    layout->addRow(tr("Order:"), m_orderSpin);

    // 截止频率
    m_cutoffSpin = new QDoubleSpinBox(panel);
    m_cutoffSpin->setObjectName(QStringLiteral("filterCutoffSpin"));
    m_cutoffSpin->setRange(1.0, 96000.0);
    m_cutoffSpin->setValue(m_params.cutoffFreq);
    m_cutoffSpin->setSuffix(tr(" Hz"));
    m_cutoffSpin->setDecimals(1);
    layout->addRow(tr("Cutoff:"), m_cutoffSpin);

    // 采样率
    m_sampleSpin = new QDoubleSpinBox(panel);
    m_sampleSpin->setObjectName(QStringLiteral("filterSampleSpin"));
    m_sampleSpin->setRange(100.0, 192000.0);
    m_sampleSpin->setValue(m_params.sampleRate);
    m_sampleSpin->setSuffix(tr(" Hz"));
    m_sampleSpin->setDecimals(0);
    layout->addRow(tr("Sample Rate:"), m_sampleSpin);

    // FIR窗类型
    m_windowCombo = new QComboBox(panel);
    m_windowCombo->setObjectName(QStringLiteral("filterWindowCombo"));
    m_windowCombo->addItem(tr("Hamming"),      static_cast<int>(WindowType::Hamming));
    m_windowCombo->addItem(tr("Hanning"),      static_cast<int>(WindowType::Hanning));
    m_windowCombo->addItem(tr("Blackman"),     static_cast<int>(WindowType::Blackman));
    m_windowCombo->addItem(tr("Rectangular"),  static_cast<int>(WindowType::Rectangular));
    m_windowCombo->setVisible(false);
    layout->addRow(tr("Window:"), m_windowCombo);

    // 相位叠加开关
    m_phaseCheck = new QCheckBox(tr("Show Phase"), panel);
    m_phaseCheck->setObjectName(QStringLiteral("filterPhaseCheck"));
    m_phaseCheck->setChecked(false);
    layout->addRow(m_phaseCheck);

    // 连接信号 (new-style connect)
    connect(m_typeCombo, &QComboBox::currentIndexChanged,
            this, &FilterDesignerWidget::onParameterChanged);
    connect(m_familyCombo, &QComboBox::currentIndexChanged,
            this, &FilterDesignerWidget::onParameterChanged);
    connect(m_orderSpin, &QSpinBox::valueChanged,
            this, &FilterDesignerWidget::onParameterChanged);
    connect(m_cutoffSpin, &QDoubleSpinBox::valueChanged,
            this, &FilterDesignerWidget::onParameterChanged);
    connect(m_sampleSpin, &QDoubleSpinBox::valueChanged,
            this, &FilterDesignerWidget::onParameterChanged);
    connect(m_windowCombo, &QComboBox::currentIndexChanged,
            this, &FilterDesignerWidget::onParameterChanged);
    connect(m_phaseCheck, &QCheckBox::toggled,
            this, [this]() { update(); });

    return panel;
}

// ============================================================
// UI 构建 — 绘图区
// ============================================================

/** @brief 创建右侧绘图区域Widget */
QWidget* FilterDesignerWidget::createPlotArea()
{
    auto* area = new QWidget(this);
    area->setObjectName(QStringLiteral("filterPlotArea"));
    return area;
}

// ============================================================
// 绘制 — 网格
// ============================================================

/** @brief 绘制背景网格 @param painter 画笔 @param plotRect 绘图区域 */
void FilterDesignerWidget::drawGrid(QPainter& painter, const QRectF& plotRect) const
{
    QPen gridPen(QColor(60, 60, 60), 1, Qt::DotLine);
    painter.setPen(gridPen);

    // 水平网格线 (dB)
    const int dbMin = -100;
    const int dbMax = 5;
    const int dbStep = 10;
    for (int db = dbMin; db <= dbMax; db += dbStep) {
        double y = plotRect.top() + plotRect.height() * (dbMax - db) / (dbMax - dbMin);
        painter.drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));
    }

    // 垂直网格线 (频率)
    const int numVLines = 10;
    for (int i = 0; i <= numVLines; ++i) {
        double x = plotRect.left() + plotRect.width() * i / numVLines;
        painter.drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));
    }

    // 边框
    QPen borderPen(QColor(100, 100, 100), 1);
    painter.setPen(borderPen);
    painter.drawRect(plotRect);
}

// ============================================================
// 绘制 — 幅度曲线
// ============================================================

/** @brief 绘制幅度响应曲线 @param painter 画笔 @param plotRect 绘图区域 */
void FilterDesignerWidget::drawMagnitudeCurve(QPainter& painter, const QRectF& plotRect) const
{
    if (m_response.freq.isEmpty() || m_response.mag.isEmpty()) {
        return;
    }

    const int dbMin = -100;
    const int dbMax = 5;
    const double nyquist = m_params.sampleRate / 2.0;

    QPainterPath path;
    bool first = true;

    for (int i = 0; i < m_response.freq.size(); ++i) {
        double x = plotRect.left() + plotRect.width() * m_response.freq[i] / nyquist;
        double magClamped = qBound(static_cast<double>(dbMin),
                                   m_response.mag[i],
                                   static_cast<double>(dbMax));
        double y = plotRect.top() + plotRect.height() * (dbMax - magClamped) / (dbMax - dbMin);

        if (first) {
            path.moveTo(x, y);
            first = false;
        } else {
            path.lineTo(x, y);
        }
    }

    QPen curvePen(QColor(0, 180, 255), 2);
    painter.setPen(curvePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}

// ============================================================
// 绘制 — 相位曲线
// ============================================================

/** @brief 绘制相位响应曲线(虚线) @param painter 画笔 @param plotRect 绘图区域 */
void FilterDesignerWidget::drawPhaseCurve(QPainter& painter, const QRectF& plotRect) const
{
    if (m_response.freq.isEmpty() || m_response.phase.isEmpty()) {
        return;
    }

    const double nyquist = m_params.sampleRate / 2.0;
    const double phMin = -360.0;
    const double phMax = 0.0;

    QPainterPath path;
    bool first = true;

    for (int i = 0; i < m_response.freq.size(); ++i) {
        double x = plotRect.left() + plotRect.width() * m_response.freq[i] / nyquist;
        double phClamped = qBound(phMin, m_response.phase[i], phMax);
        double y = plotRect.top() + plotRect.height() * (phMax - phClamped) / (phMax - phMin);

        if (first) {
            path.moveTo(x, y);
            first = false;
        } else {
            path.lineTo(x, y);
        }
    }

    QPen phasePen(QColor(255, 160, 0, 160), 1, Qt::DashLine);
    painter.setPen(phasePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}

// ============================================================
// 绘制 — 坐标轴标签
// ============================================================

/** @brief 绘制坐标轴标签 @param painter 画笔 @param plotRect 绘图区域 */
void FilterDesignerWidget::drawAxisLabels(QPainter& painter, const QRectF& plotRect) const
{
    painter.setPen(QColor(180, 180, 180));
    QFont smallFont = painter.font();
    smallFont.setPointSize(8);
    painter.setFont(smallFont);

    // Y轴标签 (dB)
    const int dbMin = -100;
    const int dbMax = 5;
    const int dbStep = 20;
    for (int db = dbMin; db <= dbMax; db += dbStep) {
        double y = plotRect.top() + plotRect.height() * (dbMax - db) / (dbMax - dbMin);
        painter.drawText(QRectF(0, y - 8, plotRect.left() - 4, 16),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(db) + tr(" dB"));
    }

    // X轴标签 (频率)
    const double nyquist = m_params.sampleRate / 2.0;
    const int numLabels = 5;
    for (int i = 0; i <= numLabels; ++i) {
        double freq = nyquist * i / numLabels;
        double x = plotRect.left() + plotRect.width() * i / numLabels;
        QString label;
        if (freq >= 1000.0) {
            label = QString::number(freq / 1000.0, 'f', 1) + tr(" kHz");
        } else {
            label = QString::number(freq, 'f', 0) + tr(" Hz");
        }
        painter.drawText(QRectF(x - 30, plotRect.bottom() + 4, 60, 16),
                         Qt::AlignCenter, label);
    }
}
