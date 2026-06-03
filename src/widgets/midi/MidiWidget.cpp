#include "widgets/midi/MidiWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>
MidiWidget::MidiWidget(QWidget *parent) : QWidget(parent) { setObjectName("MidiWidget"); setupUi(); }
MidiWidget::~MidiWidget() = default;
void MidiWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4,4,4,4); layout->setSpacing(4);
    m_statusLabel = new QLabel(tr("MIDI Monitor - Waiting..."), this); m_statusLabel->setObjectName("midiStatusLabel");
    layout->addWidget(m_statusLabel);
    m_displayLabel = new QLabel(this); m_displayLabel->setObjectName("midiDisplayLabel");
    m_displayLabel->setWordWrap(true); m_displayLabel->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    layout->addWidget(m_displayLabel);
}
void MidiWidget::appendMessage(const QByteArray &midiData) {
    if (midiData.isEmpty()) return;
    if (m_channelFilter >= 0) { int ch = midiData[0] & 0x0F; if (ch != m_channelFilter) return; }
    m_msgCount++;
    parseMidi(midiData);
    QString text = m_displayLabel->text() + formatMidi(midiData) + "\n";
    if (text.length() > 10000) text = text.right(5000);
    m_displayLabel->setText(text);
    m_statusLabel->setText(tr("MIDI Monitor - %1 messages").arg(m_msgCount));
}
void MidiWidget::setChannelFilter(int ch) { m_channelFilter = ch; }
void MidiWidget::clearDisplay() { m_displayLabel->clear(); m_msgCount = 0; m_statusLabel->setText(tr("MIDI Monitor - Waiting...")); }
int MidiWidget::messageCount() const { return m_msgCount; }
void MidiWidget::resizeEvent(QResizeEvent *event) { QWidget::resizeEvent(event); }
QString MidiWidget::formatMidi(const QByteArray &data) const {
    QString hex;
    for (auto b : data) hex += QString("%1 ").arg(static_cast<uint8_t>(b), 2, 16, QChar('0')).toUpper();
    return QString("[%1] %2").arg(m_msgCount, 5, 10, QChar('0')).arg(hex.trimmed());
}
void MidiWidget::parseMidi(const QByteArray &data) {
    if (data.size() < 1) return;
    uint8_t status = static_cast<uint8_t>(data[0]);
    uint8_t cmd = status & 0xF0; int ch = status & 0x0F;
    switch (cmd) {
    case 0x90: if (data.size() >= 3) emit noteOn(ch, data[1], data[2]); break;
    case 0x80: if (data.size() >= 3) emit noteOff(ch, data[1]); break;
    case 0xB0: if (data.size() >= 3) emit controlChange(ch, data[1], data[2]); break;
    }
    emit messageReceived(formatMidi(data));
}
