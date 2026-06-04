/**
 * @file WaveformPreviewWidget.cpp
 * @brief 波形预览控件主实现
 *
 * 包含构造函数、setupUI、feedData、parseBytes、paintEvent、
 * drawWaveform/drawGrid/drawTriggerLine绘制方法、setters及UI槽函数。
 * 统计接口见 WaveformPreviewWidgetStats.cpp。
 */

#include "chart/preview/WaveformPreviewWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QtEndian>
#include <QtMath>

#include "core/theme/ThemeManager.h"

/**
 * @brief 构造函数，初始化成员并搭建UI
 * @param parent 父控件指针
 */
WaveformPreviewWidget::WaveformPreviewWidget(QWidget* parent)
    : QWidget(parent)
    , m_maxPoints(200)
    , m_format(DataFormat::Uint8)
    , m_triggerMode(TriggerMode::Auto)
    , m_triggerLevel(0.0)
    , m_frozen(false)
    , m_yMin(0.0)
    , m_yMax(255.0)
    , m_singleTriggered(false)
    , m_infoLabel(nullptr)
    , m_freezeBtn(nullptr)
    , m_formatCombo(nullptr)
{
    m_buffer.reserve(m_maxPoints);
    setObjectName("WaveformPreviewWidget");
    setMinimumHeight(100);
    setupUI();
}

/**
 * @brief 初始化叠加层UI布局
 *
 * 右上角: 格式下拉框 + 冻结按钮
 * 左下角: 信息标签(当前值/Y范围/点数)
 */
void WaveformPreviewWidget::setupUI()
{
    /* 使用QHBoxLayout作为主布局，内部嵌套实现叠加效果 */
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(0);

    /* 左下角信息标签 */
    m_infoLabel = new QLabel(tr("No data"), this);
    m_infoLabel->setObjectName("WaveformPreviewInfoLabel");
    m_infoLabel->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    QFont infoFont = m_infoLabel->font();
    infoFont.setPointSize(8);
    m_infoLabel->setFont(infoFont);

    mainLayout->addWidget(m_infoLabel, 1);

    /* 右上角控件容器 */
    auto* ctrlLayout = new QVBoxLayout();
    ctrlLayout->setSpacing(2);

    /* 格式下拉框 */
    m_formatCombo = new QComboBox(this);
    m_formatCombo->setObjectName("WaveformPreviewFormatCombo");
    m_formatCombo->addItem("Uint8", static_cast<int>(DataFormat::Uint8));
    m_formatCombo->addItem("Int16LE", static_cast<int>(DataFormat::Int16LE));
    m_formatCombo->addItem("Int16BE", static_cast<int>(DataFormat::Int16BE));
    m_formatCombo->addItem("Uint16LE", static_cast<int>(DataFormat::Uint16LE));
    m_formatCombo->addItem("Float32LE", static_cast<int>(DataFormat::Float32LE));
    m_formatCombo->setFixedWidth(100);

    /* 冻结按钮 */
    m_freezeBtn = new QPushButton(tr("Freeze"), this);
    m_freezeBtn->setObjectName("WaveformPreviewFreezeBtn");
    m_freezeBtn->setCheckable(true);
    m_freezeBtn->setFixedWidth(70);

    ctrlLayout->addWidget(m_formatCombo, 0, Qt::AlignRight | Qt::AlignTop);
    ctrlLayout->addWidget(m_freezeBtn, 0, Qt::AlignRight | Qt::AlignTop);
    ctrlLayout->addStretch(1);

    mainLayout->addLayout(ctrlLayout, 0);

    /* 信号连接 — 新式connect语法 */
    connect(m_formatCombo, &QComboBox::currentIndexChanged,
            this, &WaveformPreviewWidget::onFormatChanged);
    connect(m_freezeBtn, &QPushButton::toggled,
            this, &WaveformPreviewWidget::onFreezeToggled);
}

/**
 * @brief 喂入原始字节数据
 *
 * 冻结状态下丢弃数据。否则调用parseBytes解析，然后
 * 检查触发条件，更新统计，触发重绘。
 * @param data 原始字节数据
 */
void WaveformPreviewWidget::feedData(const QByteArray& data)
{
    if (m_frozen || data.isEmpty()) {
        return;
    }

    m_stats.totalBytesFed += static_cast<quint64>(data.size());
    parseBytes(data);
    computeYRange();
    update();
}

/**
 * @brief 将原始字节按m_format解析为double追加到缓冲区
 *
 * 根据格式读取1/2/4字节转换为double，缓冲区满时FIFO移除最旧数据。
 * 同时更新统计峰值/最小值，检查触发条件。
 * @param data 原始字节数据
 */
void WaveformPreviewWidget::parseBytes(const QByteArray& data)
{
    const char* ptr = data.constData();
    int remaining = data.size();
    int step = 1;

    /* 每种格式的字节步进 */
    switch (m_format) {
    case DataFormat::Uint8:     step = 1; break;
    case DataFormat::Int16LE:   step = 2; break;
    case DataFormat::Int16BE:   step = 2; break;
    case DataFormat::Uint16LE:  step = 2; break;
    case DataFormat::Float32LE: step = 4; break;
    }

    while (remaining >= step) {
        double value = 0.0;
        switch (m_format) {
        case DataFormat::Uint8:
            value = static_cast<double>(static_cast<uint8_t>(*ptr)); break;
        case DataFormat::Int16LE:
            value = static_cast<double>(qFromLittleEndian<int16_t>(ptr)); break;
        case DataFormat::Int16BE:
            value = static_cast<double>(qFromBigEndian<int16_t>(ptr)); break;
        case DataFormat::Uint16LE:
            value = static_cast<double>(qFromLittleEndian<uint16_t>(ptr)); break;
        case DataFormat::Float32LE:
            value = static_cast<double>(qFromLittleEndian<float>(ptr)); break;
        }

        /* 触发检查: Normal/Single模式下等待上升沿穿越 */
        bool triggered = true;
        if (m_triggerMode == TriggerMode::Normal
            || m_triggerMode == TriggerMode::Single) {
            double prev = m_buffer.isEmpty() ? 0.0 : m_buffer.last();
            triggered = (prev < m_triggerLevel && value >= m_triggerLevel);
            if (m_triggerMode == TriggerMode::Single && m_singleTriggered)
                triggered = false;
        }

        if (triggered) {
            m_buffer.append(value);
            ++m_stats.totalPointsReceived;
            if (value > m_stats.peakValue || !m_stats.hasValue)
                m_stats.peakValue = value;
            if (value < m_stats.minValue || !m_stats.hasValue)
                m_stats.minValue = value;
            m_stats.hasValue = true;

            if (m_triggerMode != TriggerMode::Auto) {
                emit triggerFired(m_triggerLevel);
                ++m_stats.totalTriggersFired;
                if (m_triggerMode == TriggerMode::Single)
                    m_singleTriggered = true;
            }
            emit pointReceived(value);
        }

        while (m_buffer.size() > m_maxPoints)
            m_buffer.removeFirst();

        ptr += step;
        remaining -= step;
    }
}

/**
 * @brief 根据缓冲区数据计算Y轴自动范围(含10%边距)
 */
void WaveformPreviewWidget::computeYRange()
{
    if (m_buffer.isEmpty()) {
        m_yMin = 0.0;
        m_yMax = 255.0;
        return;
    }

    double lo = std::numeric_limits<double>::max();
    double hi = std::numeric_limits<double>::lowest();
    for (double v : m_buffer) {
        if (v < lo) lo = v;
        if (v > hi) hi = v;
    }

    /* 所有值相同时扩展范围 */
    if (qFuzzyCompare(lo, hi)) {
        lo -= 1.0;
        hi += 1.0;
    }

    /* 上下各留10%边距 */
    double range = hi - lo;
    double margin = range * 0.1;
    m_yMin = lo - margin;
    m_yMax = hi + margin;
}

/**
 * @brief 绘制事件 — 依次绘制网格、触发线、波形、更新信息标签
 * @param event 绘制事件参数
 */
void WaveformPreviewWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    ++m_stats.totalFramesDisplayed;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    /* 背景 */
    auto& theme = ThemeManager::instance();
    painter.setPen(Qt::NoPen);
    painter.setBrush(theme.color(ThemeManager::SemanticColor::BgSecondary));
    painter.drawRect(rect());

    drawGrid(&painter);
    drawTriggerLine(&painter);
    drawWaveform(&painter);

    /* 更新信息标签 */
    if (!m_buffer.isEmpty()) {
        double last = m_buffer.last();
        m_infoLabel->setText(
            tr("Value: %1 | Range: %2 ~ %3 | Points: %4")
                .arg(last, 0, 'f', 2)
                .arg(m_yMin, 0, 'f', 1)
                .arg(m_yMax, 0, 'f', 1)
                .arg(m_buffer.size()));
    }

    painter.end();
}

/**
 * @brief 绘制网格线(5水平 + 5垂直分割)
 * @param painter 画笔指针
 */
void WaveformPreviewWidget::drawGrid(QPainter* painter)
{
    auto& theme = ThemeManager::instance();
    QPen gridPen(theme.color(ThemeManager::SemanticColor::Border));
    gridPen.setStyle(Qt::DotLine);
    gridPen.setWidthF(0.6);
    painter->setPen(gridPen);

    QRectF r = rect();
    int divisions = 5;

    /* 水平网格线 */
    for (int i = 1; i < divisions; ++i) {
        qreal y = r.top() + (r.height() * i) / divisions;
        painter->drawLine(QPointF(r.left(), y), QPointF(r.right(), y));
    }

    /* 垂直网格线 */
    for (int i = 1; i < divisions; ++i) {
        qreal x = r.left() + (r.width() * i) / divisions;
        painter->drawLine(QPointF(x, r.top()), QPointF(x, r.bottom()));
    }
}

/**
 * @brief 绘制触发模式下的虚线触发电平线
 * @param painter 画笔指针
 */
void WaveformPreviewWidget::drawTriggerLine(QPainter* painter)
{
    if (m_triggerMode == TriggerMode::Auto) {
        return;
    }

    double yRange = m_yMax - m_yMin;
    if (qFuzzyIsNull(yRange)) {
        return;
    }

    auto& theme = ThemeManager::instance();
    QPen triggerPen(theme.color(ThemeManager::SemanticColor::Warning));
    triggerPen.setStyle(Qt::DashLine);
    triggerPen.setWidthF(1.0);
    painter->setPen(triggerPen);

    QRectF r = rect();
    qreal normalizedY = (m_triggerLevel - m_yMin) / yRange;
    qreal y = r.bottom() - normalizedY * r.height();
    y = qBound(r.top(), y, r.bottom());

    painter->drawLine(QPointF(r.left(), y), QPointF(r.right(), y));

    /* 触发电平数值标注 */
    QFont smallFont = font();
    smallFont.setPointSize(7);
    painter->setFont(smallFont);
    painter->setPen(theme.color(ThemeManager::SemanticColor::Warning));
    painter->drawText(QPointF(r.left() + 4, y - 2),
                      QString("T: %1").arg(m_triggerLevel, 0, 'f', 1));
}

/**
 * @brief 绘制波形折线(绿色，抗锯齿)
 * @param painter 画笔指针
 */
void WaveformPreviewWidget::drawWaveform(QPainter* painter)
{
    if (m_buffer.size() < 2 || m_maxPoints <= 1) {
        return;
    }

    auto& theme = ThemeManager::instance();
    QRectF r = rect();
    double yRange = m_yMax - m_yMin;
    if (qFuzzyIsNull(yRange)) {
        yRange = 1.0;
    }

    /* 构建折线路径 */
    QPainterPath path;
    int count = m_buffer.size();
    qreal stepX = r.width() / static_cast<qreal>(m_maxPoints - 1);

    /* 计算起始X偏移，使最新数据靠右 */
    qreal startX = r.right() - (count - 1) * stepX;

    for (int i = 0; i < count; ++i) {
        qreal x = startX + i * stepX;
        qreal normalizedY = (m_buffer[i] - m_yMin) / yRange;
        qreal y = r.bottom() - normalizedY * r.height();
        y = qBound(r.top(), y, r.bottom());

        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    /* 波形下方半透明渐变填充 */
    QPainterPath fillPath = path;
    fillPath.lineTo(r.right(), r.bottom());
    fillPath.lineTo(startX, r.bottom());
    fillPath.closeSubpath();

    QColor accent = theme.color(ThemeManager::SemanticColor::Success);
    QLinearGradient gradient(0, r.top(), 0, r.bottom());
    gradient.setColorAt(0.0, QColor(accent.red(), accent.green(),
                                     accent.blue(), 60));
    gradient.setColorAt(1.0, QColor(accent.red(), accent.green(),
                                     accent.blue(), 5));
    painter->setPen(Qt::NoPen);
    painter->setBrush(gradient);
    painter->drawPath(fillPath);

    /* 波形折线描边 */
    QPen linePen(accent, 1.5);
    linePen.setCapStyle(Qt::RoundCap);
    linePen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(linePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);

    /* 末端数据点圆点标记 */
    if (!m_buffer.isEmpty()) {
        qreal lastY = r.bottom()
            - ((m_buffer.last() - m_yMin) / yRange) * r.height();
        lastY = qBound(r.top(), lastY, r.bottom());
        painter->setPen(Qt::NoPen);
        painter->setBrush(accent);
        painter->drawEllipse(QPointF(r.right(), lastY), 3.0, 3.0);
    }
}

/**
 * @brief 设置数据解析格式
 * @param format 目标格式
 */
void WaveformPreviewWidget::setDataFormat(DataFormat format)
{
    m_format = format;
    /* 同步下拉框(不触发信号) */
    m_formatCombo->blockSignals(true);
    m_formatCombo->setCurrentIndex(static_cast<int>(format));
    m_formatCombo->blockSignals(false);
}

/**
 * @brief 设置最大显示点数
 * @param points 最大点数(会被钳制到>=10)
 */
void WaveformPreviewWidget::setMaxPoints(int points)
{
    m_maxPoints = qMax(10, points);
    while (m_buffer.size() > m_maxPoints) {
        m_buffer.removeFirst();
    }
    m_buffer.reserve(m_maxPoints);
    update();
}

/**
 * @brief 冻结或恢复显示
 * @param freeze true=冻结 false=恢复
 */
void WaveformPreviewWidget::setFreeze(bool freeze)
{
    m_frozen = freeze;
    m_freezeBtn->blockSignals(true);
    m_freezeBtn->setChecked(freeze);
    m_freezeBtn->blockSignals(false);
    m_freezeBtn->setText(freeze ? tr("Resume") : tr("Freeze"));
    if (freeze) {
        ++m_stats.totalFreezeCount;
    }
    /* Single触发恢复时重置触发状态 */
    if (!freeze && m_triggerMode == TriggerMode::Single) {
        m_singleTriggered = false;
    }
    update();
}

/**
 * @brief 设置触发模式和触发电平
 * @param mode 触发模式
 * @param level 触发电平
 */
void WaveformPreviewWidget::setTrigger(TriggerMode mode, double level)
{
    m_triggerMode = mode;
    m_triggerLevel = level;
    m_singleTriggered = false;
    update();
}

/**
 * @brief 格式下拉框选项变更处理
 * @param index 新选项索引
 */
void WaveformPreviewWidget::onFormatChanged(int index)
{
    if (index >= 0 && index <= static_cast<int>(DataFormat::Float32LE)) {
        m_format = static_cast<DataFormat>(index);
    }
}

/**
 * @brief 冻结按钮点击切换处理
 */
void WaveformPreviewWidget::onFreezeToggled()
{
    setFreeze(m_freezeBtn->isChecked());
}
