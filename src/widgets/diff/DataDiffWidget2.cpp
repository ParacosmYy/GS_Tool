#include "widgets/diff/DataDiffWidget2.h"
#include <QPainter>
DataDiffWidget::DataDiffWidget(QWidget *parent) : QWidget(parent) { setObjectName("DataDiffWidget2"); }
DataDiffWidget::~DataDiffWidget() = default;
void DataDiffWidget::setLeftData(const QByteArray &d) { m_left = d; update(); }
void DataDiffWidget::setRightData(const QByteArray &d) { m_right = d; update(); }
void DataDiffWidget::setBytesPerLine(int b) { m_bytesPerLine = b; update(); }
void DataDiffWidget::clear() { m_left.clear(); m_right.clear(); m_diffCount = 0; update(); }
int DataDiffWidget::diffCount() const { return m_diffCount; }

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
