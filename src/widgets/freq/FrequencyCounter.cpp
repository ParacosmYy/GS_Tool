#include "widgets/freq/FrequencyCounter.h"
#include <QVBoxLayout>
#include <QDateTime>

FrequencyCounter::FrequencyCounter(QWidget *parent) : QWidget(parent), m_gateTimer(new QTimer(this)) {
    setObjectName("FrequencyCounter");
    setupUi();
    connect(m_gateTimer, &QTimer::timeout, this, &FrequencyCounter::onGateTimeout);
}
FrequencyCounter::~FrequencyCounter() = default;

void FrequencyCounter::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4,4,4,4);
    m_freqLabel = new QLabel(tr("0.000 Hz"), this);
    m_freqLabel->setObjectName("freqValueLabel");
    m_freqLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_freqLabel);
    m_countLabel = new QLabel(tr("Pulses: 0"), this);
    m_countLabel->setObjectName("freqCountLabel");
    m_countLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_countLabel);
}

void FrequencyCounter::feedSample(double value) {
    bool above = value >= m_triggerLevel;
    if (above && !m_lastAbove) {
        m_pulseCount++;
        m_countLabel->setText(tr("Pulses: %1").arg(m_pulseCount));
        emit pulseCounted(m_pulseCount);
    }
    m_lastAbove = above;
    if (!m_gateTimer->isActive()) {
        m_gateStart = QDateTime::currentMSecsSinceEpoch();
        m_pulseCount = 0;
        m_gateTimer->start(m_gateTime);
    }
}

void FrequencyCounter::feedBuffer(const QByteArray &data, int sampleSize) {
    for (int i = 0; i + sampleSize <= data.size(); i += sampleSize) {
        double val = 0;
        if (sampleSize == 2) {
            qint16 s; memcpy(&s, data.constData()+i, 2);
            val = static_cast<double>(s);
        } else if (sampleSize == 4) {
            float f; memcpy(&f, data.constData()+i, 4);
            val = static_cast<double>(f);
        }
        feedSample(val);
    }
}

void FrequencyCounter::setGateTime(int ms) { m_gateTime = ms; }
void FrequencyCounter::setTriggerLevel(double l) { m_triggerLevel = l; }
void FrequencyCounter::reset() { m_pulseCount = 0; m_frequency = 0; m_freqLabel->setText(tr("0.000 Hz")); m_countLabel->setText(tr("Pulses: 0")); }
double FrequencyCounter::frequency() const { return m_frequency; }
double FrequencyCounter::period() const { return m_frequency > 0 ? 1.0 / m_frequency : 0.0; }
int FrequencyCounter::pulseCount() const { return m_pulseCount; }

void FrequencyCounter::onGateTimeout() {
    m_gateTimer->stop();
    double elapsed = static_cast<double>(QDateTime::currentMSecsSinceEpoch() - m_gateStart) / 1000.0;
    if (elapsed > 0 && m_pulseCount > 0) {
        m_frequency = m_pulseCount / elapsed;
        m_freqLabel->setText(tr("%1 Hz").arg(m_frequency, 0, 'f', 3));
        emit frequencyChanged(m_frequency);
    }
}
