/**
 * @file SvdBitFieldWidget.cpp
 * @brief SVD位字段可视化控件实现 — 绘制/鼠标交互/数据管理
 *
 * 包含构造/析构、setRegister/setFieldValue/clear、paintEvent自定义绘制、
 * 鼠标事件处理(悬停/点击)、颜色映射和命中测试辅助方法。
 * 统计重置在 SvdViewerWidgetStats.cpp。
 */

#include "protocol/svd/SvdBitFieldWidget.h"
#include "protocol/svd/SvdTypes.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QFontMetrics>

// ============================================================================
// 构造
// ============================================================================

/** @brief 构造位字段可视化控件，启用鼠标追踪 @param parent 父控件 */
SvdBitFieldWidget::SvdBitFieldWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("SvdBitFieldWidget");
    setMinimumHeight(kRowHeight);
    setMouseTracking(true);
}

// ============================================================================
// 数据设置
// ============================================================================

/** @brief 设置寄存器数据，提取字段并触发重绘 @param reg SVD寄存器结构体 */
void SvdBitFieldWidget::setRegister(const SvdRegister& reg)
{
    m_fields.clear();
    m_totalBits = reg.size;
    m_registerValue = static_cast<quint32>(reg.resetValue);

    for (const SvdField& field : reg.fields) {
        FieldItem item;
        item.name = field.name;
        item.bitOffset = field.bitOffset;
        item.bitWidth = field.bitWidth;
        item.access = field.access;
        item.description = field.description;
        m_fields.append(item);
    }

    m_selectedIndex = -1;
    m_hoveredIndex = -1;
    update();
}

/** @brief 设置寄存器总位宽 @param bits 位宽(8/16/32) */
void SvdBitFieldWidget::setTotalBits(int bits)
{
    m_totalBits = qMax(1, bits);
    update();
}

/** @brief 设置寄存器当前值(更新字段值显示) @param value 寄存器值 */
void SvdBitFieldWidget::setFieldValue(quint32 value)
{
    m_registerValue = value;
    update();
}

/** @brief 清除所有字段和状态 */
void SvdBitFieldWidget::clear()
{
    m_fields.clear();
    m_totalBits = 32;
    m_registerValue = 0;
    m_selectedIndex = -1;
    m_hoveredIndex = -1;
    update();
}

/** @brief 获取当前选中字段索引 @return 索引，-1表示无选中 */
int SvdBitFieldWidget::selectedFieldIndex() const
{
    return m_selectedIndex;
}

// ============================================================================
// 绘制
// ============================================================================

/** @brief 自定义绘制事件 — 刻度标尺 + 字段块 + 悬停/选中高亮 + 保留位 @param event 绘制事件 */
void SvdBitFieldWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    ++m_totalPaintCount;

    const int drawWidth = width() - 2 * kSideMargin;
    if (drawWidth <= 0 || m_totalBits <= 0) {
        painter.setPen(palette().color(QPalette::Text));
        painter.drawText(rect(), Qt::AlignCenter,
                         tr("未选择寄存器"));
        painter.end();
        return;
    }

    const double pxPerBit = static_cast<double>(drawWidth) / m_totalBits;
    const int blockY = kTopMargin;
    const int blockHeight = height() - kTopMargin - 4;

    /* --- 1. 绘制刻度标尺(位号) --- */
    painter.setPen(palette().color(QPalette::Text));
    QFont rulerFont = font();
    rulerFont.setPointSize(qMax(7, font().pointSize() - 1));
    painter.setFont(rulerFont);

    QFontMetrics fm(rulerFont);
    int step = 1;
    if (pxPerBit < 6.0) { step = 8; }
    else if (pxPerBit < 12.0) { step = 4; }
    else if (pxPerBit < 20.0) { step = 2; }

    for (int i = 0; i <= m_totalBits; i += step) {
        /* 位号从高位(MSB)到低位(LSB)排列 */
        int bitNum = m_totalBits - 1 - i;
        double x = kSideMargin + i * pxPerBit;
        if (x < kSideMargin - 10 || x > width()) { continue; }

        painter.setPen(palette().color(QPalette::Mid));
        painter.drawLine(static_cast<int>(x), 0,
                         static_cast<int>(x), kTopMargin - 2);

        painter.setPen(palette().color(QPalette::Text));
        QRect labelRect(static_cast<int>(x) - 15, 0, 30, kTopMargin - 2);
        painter.drawText(labelRect, Qt::AlignCenter, QString::number(bitNum));
    }

    /* --- 2. 绘制保留位底色(灰色) --- */
    QRectF fullRect(kSideMargin, blockY, drawWidth, blockHeight);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().color(QPalette::Window));
    painter.drawRoundedRect(fullRect, 3.0, 3.0);

    /* --- 3. 绘制字段着色块 --- */
    for (int i = 0; i < m_fields.size(); ++i) {
        const auto& field = m_fields[i];
        double fieldX = kSideMargin + field.bitOffset * pxPerBit;
        double fieldW = field.bitWidth * pxPerBit;

        /* 跳过不可见字段 */
        if (fieldX + fieldW < kSideMargin || fieldX > width()) { continue; }

        QColor color = accessColor(field.access);

        /* 绘制字段矩形 */
        QRectF blockRect(fieldX, blockY, fieldW, blockHeight);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRoundedRect(blockRect, 2.0, 2.0);

        /* 悬停高亮叠加 */
        if (i == m_hoveredIndex) {
            painter.setBrush(QColor(255, 255, 255, 50));
            painter.drawRoundedRect(blockRect, 2.0, 2.0);
        }

        /* 选中边框 */
        if (i == m_selectedIndex) {
            QPen selPen(palette().color(QPalette::Highlight), 2.0);
            painter.setPen(selPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(blockRect.adjusted(1, 1, -1, -1),
                                    2.0, 2.0);
        }

        /* 字段内文本: 名称 */
        painter.setPen(palette().color(QPalette::Text));
        QFont textFont = font();
        int fontSize = qMax(7, static_cast<int>(blockHeight * 0.22));
        textFont.setPointSize(fontSize);
        painter.setFont(textFont);

        QFontMetrics textFm(textFont);
        QString nameText = field.name;
        if (textFm.horizontalAdvance(nameText) > fieldW - 4) {
            nameText = textFm.elidedText(nameText, Qt::ElideRight,
                                          static_cast<int>(fieldW - 4));
        }
        QRectF nameRect(fieldX + 2, blockY + 2,
                         fieldW - 4, blockHeight * 0.5);
        painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignBottom, nameText);

        /* 字段值(第二行) */
        if (fieldW > 18) {
            quint32 mask = (field.bitWidth >= 32)
                ? 0xFFFFFFFF
                : ((1U << field.bitWidth) - 1);
            quint32 val = (m_registerValue >> field.bitOffset) & mask;
            QString valStr = QStringLiteral("0x%1")
                .arg(val, (field.bitWidth + 3) / 4, 16, QChar('0')).toUpper();

            QFont smallFont = textFont;
            smallFont.setPointSize(qMax(6, fontSize - 1));
            painter.setFont(smallFont);
            QFontMetrics smallFm(smallFont);
            if (smallFm.horizontalAdvance(valStr) > fieldW - 4) {
                valStr = smallFm.elidedText(valStr, Qt::ElideMiddle,
                                             static_cast<int>(fieldW - 4));
            }
            QRectF valRect(fieldX + 2, blockY + blockHeight * 0.5,
                            fieldW - 4, blockHeight * 0.45);
            painter.drawText(valRect, Qt::AlignLeft | Qt::AlignTop, valStr);
        }
    }

    painter.end();
}

// ============================================================================
// 鼠标事件
// ============================================================================

/** @brief 鼠标移动事件 — 检测悬停字段并显示提示 @param event 鼠标事件 */
void SvdBitFieldWidget::mouseMoveEvent(QMouseEvent* event)
{
    int newHovered = hitTestField(event->pos().x());

    if (newHovered != m_hoveredIndex) {
        m_hoveredIndex = newHovered;
        ++m_totalFieldHoverCount;
        emit fieldHovered(newHovered);
        update();

        if (newHovered >= 0 && newHovered < m_fields.size()) {
            QString tip = fieldTooltip(m_fields[newHovered]);
            QToolTip::showText(event->globalPosition().toPoint(), tip, this);
        } else {
            QToolTip::hideText();
        }
    }
}

/** @brief 鼠标点击事件 — 选中字段并发射信号 @param event 鼠标事件 */
void SvdBitFieldWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int clickedIndex = hitTestField(event->pos().x());
        if (clickedIndex >= 0) {
            m_selectedIndex = clickedIndex;
            ++m_totalFieldClickCount;
            emit fieldClicked(clickedIndex);
        } else {
            m_selectedIndex = -1;
        }
        update();
    }
}

/** @brief 建议最小尺寸 @return 宽200 x 高kRowHeight */
QSize SvdBitFieldWidget::minimumSizeHint() const
{
    return QSize(200, kRowHeight);
}

/** @brief 建议尺寸 @return 宽400 x 高kRowHeight */
QSize SvdBitFieldWidget::sizeHint() const
{
    return QSize(400, kRowHeight);
}

// ============================================================================
// 私有辅助方法
// ============================================================================

/** @brief 根据访问类型返回对应颜色 @param access 访问类型字符串 @return 颜色 */
QColor SvdBitFieldWidget::accessColor(const QString& access)
{
    QString acc = access.toLower();
    if (acc.contains("read") && acc.contains("write")) {
        return QColor(255, 165, 0, 180);   ///< 橙色: read-write
    }
    if (acc.contains("read")) {
        return QColor(70, 130, 180, 180);  ///< 钢蓝色: read-only
    }
    if (acc.contains("write")) {
        return QColor(60, 179, 113, 180);  ///< 中等海洋绿: write-only
    }
    return QColor(169, 169, 169, 180);     ///< 深灰色: 未知
}

/** @brief 根据像素X坐标计算命中的字段索引 @param pixelX 像素X坐标 @return 字段索引，-1未命中 */
int SvdBitFieldWidget::hitTestField(int pixelX) const
{
    if (m_fields.isEmpty() || m_totalBits <= 0) { return -1; }

    const int drawWidth = width() - 2 * kSideMargin;
    const double pxPerBit = static_cast<double>(drawWidth) / m_totalBits;

    double adjustedX = pixelX - kSideMargin;
    if (adjustedX < 0) { return -1; }

    double bitPos = adjustedX / pxPerBit;

    for (int i = 0; i < m_fields.size(); ++i) {
        const auto& field = m_fields[i];
        if (bitPos >= field.bitOffset &&
            bitPos < (field.bitOffset + field.bitWidth)) {
            return i;
        }
    }
    return -1;
}

/** @brief 生成字段的悬停提示文本 @param field 字段数据 @return HTML格式提示 */
QString SvdBitFieldWidget::fieldTooltip(const FieldItem& field) const
{
    quint32 mask = (field.bitWidth >= 32)
        ? 0xFFFFFFFF
        : ((1U << field.bitWidth) - 1);
    quint32 val = (m_registerValue >> field.bitOffset) & mask;

    return tr("<b>%1</b><br>"
              "位范围: [%2:%3]<br>"
              "位宽: %4<br>"
              "访问: %5<br>"
              "当前值: 0x%6<br>"
              "%7")
        .arg(field.name)
        .arg(field.bitOffset + field.bitWidth - 1)
        .arg(field.bitOffset)
        .arg(field.bitWidth)
        .arg(field.access)
        .arg(val, (field.bitWidth + 3) / 4, 16, QChar('0')).toUpper()
        .arg(field.description);
}

// 统计重置见 SvdViewerWidgetStats.cpp
