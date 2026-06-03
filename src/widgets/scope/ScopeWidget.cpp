#include "widgets/scope/ScopeWidget.h"
#include <QPainter>
#include <QResizeEvent>
#include <QtMath>

ScopeWidget::ScopeWidget(QWidget *parent) : QWidget(parent) { setObjectName("ScopeWidget"); setChannelCount(2); }
ScopeWidget::~ScopeWidget() = default;

void ScopeWidget::setChannelCount(int c) { m_channels.resize(c); for (auto &ch : m_channels) ch.resize(m_bufferSize); }
void ScopeWidget::setSampleBuffer(int s) { m_bufferSize = s; for (auto &ch : m_channels) ch.resize(s); m_writePos = 0; }
void ScopeWidget::addSample(int ch, double v) { if (ch >= 0 && ch < m_channels.size()) { m_channels[ch][m_writePos % m_bufferSize] = v; if (ch == 0) { m_writePos++; if (m_writePos >= m_bufferSize) { m_writePos = 0; emit dataOverflow(); } } } }
void ScopeWidget::addSamples(int ch, const QVector<double> &vals) { for (auto v : vals) addSample(ch, v); }
void ScopeWidget::setTimeScale(double ms) { m_timeScale = ms; }
void ScopeWidget::setVoltageScale(double v) { m_voltScale = v; }
void ScopeWidget::setTriggerChannel(int c) { m_triggerCh = c; }
void ScopeWidget::setTriggerLevel(double l) { m_triggerLevel = l; }
void ScopeWidget::setRunning(bool on) { m_running = on; }
void ScopeWidget::clearData() { for (auto &ch : m_channels) ch.fill(0); m_writePos = 0; }
int ScopeWidget::channelCount() const { return m_channels.size(); }
bool ScopeWidget::isRunning() const { return m_running; }

void ScopeWidget::paintEvent(QPaintEvent *) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    int w = width(), h = height();
    p.fillRect(rect(), QColor(20, 20, 30));
    drawGrid(p, w, h);
    static const QColor colors[] = { QColor(0,255,0), QColor(255,255,0), QColor(0,200,255), QColor(255,100,100) };
    for (int c = 0; c < m_channels.size(); ++c) {
        p.setPen(QPen(colors[c % 4], 1.5));
        int drawLen = qMin(m_writePos, m_bufferSize);
        if (drawLen < 2) continue;
        QPainterPath path;
        for (int i = 0; i < drawLen; ++i) {
            double x = static_cast<double>(i) / drawLen * w;
            double y = h / 2.0 - m_channels[c][i] / m_voltScale * (h / kDivisions);
            if (i == 0) path.moveTo(x, y); else path.lineTo(x, y);
        }
        p.drawPath(path);
    }
}

void ScopeWidget::drawGrid(QPainter &p, int w, int h) {
    p.setPen(QPen(QColor(60, 60, 80), 1, Qt::DotLine));
    for (int i = 1; i < kDivisions; ++i) {
        int x = i * w / kDivisions;
        p.drawLine(x, 0, x, h);
    }
    for (int i = 1; i < kDivisions; ++i) {
        int y = i * h / kDivisions;
        p.drawLine(0, y, w, y);
    }
    p.setPen(QPen(QColor(100, 100, 120), 1));
    p.drawLine(w/2, 0, w/2, h);
    p.drawLine(0, h/2, w, h/2);
}

void ScopeWidget::resizeEvent(QResizeEvent *e) { QWidget::resizeEvent(e); update(); }
