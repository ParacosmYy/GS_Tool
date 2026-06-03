/**
 * @file DataDiffWidget2.cpp
 * @brief 数据对比组件v2实现 — 二进制数据逐行对比+差异高亮绘制
 */
#include "widgets/diff/DataDiffWidget2.h"
#include <QPainter>

/** @brief 构造函数 @param parent 父Widget */
DataDiffWidget::DataDiffWidget(QWidget *parent) : QWidget(parent) { setObjectName("DataDiffWidget2"); }
/** @brief 析构函数 */
DataDiffWidget::~DataDiffWidget() = default;
/** @brief 设置左侧对比数据 @param d 左侧字节数组 */
void DataDiffWidget::setLeftData(const QByteArray &d) { m_left = d; update(); }
/** @brief 设置右侧对比数据 @param d 右侧字节数组 */
void DataDiffWidget::setRightData(const QByteArray &d) { m_right = d; update(); }
/** @brief 设置每行显示字节数 @param b 字节数 */
void DataDiffWidget::setBytesPerLine(int b) { m_bytesPerLine = b; update(); }
/** @brief 清空两侧数据并重置差异计数 */
void DataDiffWidget::clear() { m_left.clear(); m_right.clear(); m_diffCount = 0; update(); }
/** @brief 获取差异数量 @return 不匹配的行数 */
int DataDiffWidget::diffCount() const { return m_diffCount; }

/** @brief 计算逐行差异列表 @return DiffLine列表 */
QList<DataDiffWidget::DiffLine> DataDiffWidget::computeDiff() const {
    QList<DiffLine> result;
    int lines = qMax((m_left.size()+m_bytesPerLine-1)/m_bytesPerLine, (m_right.size()+m_bytesPerLine-1)/m_bytesPerLine);
    m_diffCount = 0;
    for (int i = 0; i < lines; ++i) {
        DiffLine dl;
        dl.leftLine = i; dl.rightLine = i;
        dl.leftData = m_left.mid(i*m_bytesPerLine, m_bytesPerLine);
        dl.rightData = m_right.mid(i*m_bytesPerLine, m_bytesPerLine);
        dl.different = (dl.leftData != dl.rightData);
        if (dl.different) m_diffCount++;
        result.append(dl);
    }
    return result;
}

/** @brief 绘制对比视图 — 差异行红色背景高亮 */
void DataDiffWidget::paintEvent(QPaintEvent *) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing, false);
    auto diffs = computeDiff();
    int y = 0; QFontMetrics fm(font());
    for (const auto &dl : diffs) {
        if (dl.different) { p.setBackgroundMode(Qt::OpaqueMode); p.setBackground(QColor(255,200,200)); }
        else { p.setBackgroundMode(Qt::TransparentMode); }
        p.drawText(0, y + fm.ascent(), dl.leftData.toHex(' ') + " | " + dl.rightData.toHex(' '));
        y += fm.height();
    }
    emit diffComputed(m_diffCount, diffs.size());
}
