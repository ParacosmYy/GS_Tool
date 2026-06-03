#include "utils/signalgen/SignalGenerator.h"
#include <QtMath>
#include <QRandomGenerator>

SignalGenerator::SignalGenerator(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)) {
    connect(m_timer, &QTimer::timeout, this, &SignalGenerator::onTimerTick);
}

SignalGenerator::~SignalGenerator() { stop(); }

void SignalGenerator::setWaveform(WaveformType t) { m_waveform = t; }
void SignalGenerator::setFrequency(double hz) { m_frequency = hz; }
void SignalGenerator::setAmplitude(double a) { m_amplitude = a; }
void SignalGenerator::setOffset(double o) { m_offset = o; }
void SignalGenerator::setSampleRate(int r) { m_sampleRate = r; }
void SignalGenerator::setDuration(int ms) { m_duration = ms; }
void SignalGenerator::setCustomData(const QByteArray &d) { m_customData = d; }

void SignalGenerator::start() {
    m_sampleIndex = 0;
    m_running = true;
    int interval = m_sampleRate > 0 ? 1000 / m_sampleRate : 10;
    m_timer->start(interval);
}

void SignalGenerator::stop() {
    m_timer->stop();
    m_running = false;
    emit generationFinished(m_sampleIndex);
}

bool SignalGenerator::isRunning() const { return m_running; }

double SignalGenerator::currentValue() const {
    return computeSample(m_sampleIndex);
}

QByteArray SignalGenerator::generateSamples(int count) {
    QByteArray buf;
    for (int i = 0; i < count; ++i) {
        double val = computeSample(i);
        buf.append(reinterpret_cast<const char *>(&val), sizeof(double));
    }
    return buf;
}

QByteArray SignalGenerator::generateDuration(int msecs) {
    int count = m_sampleRate * msecs / 1000;
    return generateSamples(count);
}

SignalGenerator::WaveformType SignalGenerator::waveform() const { return m_waveform; }
double SignalGenerator::frequency() const { return m_frequency; }
double SignalGenerator::amplitude() const { return m_amplitude; }
int SignalGenerator::sampleRate() const { return m_sampleRate; }

void SignalGenerator::onTimerTick() {
    double val = computeSample(m_sampleIndex);
    emit sampleReady(val);
    m_sampleIndex++;
    if (m_duration > 0 && m_sampleIndex >= m_sampleRate * m_duration / 1000) stop();
}

double SignalGenerator::computeSample(int idx) const {
    if (m_waveform == Custom && !m_customData.isEmpty()) {
        int byteIdx = idx * sizeof(double);
        if (byteIdx + sizeof(double) <= m_customData.size())
            return *reinterpret_cast<const double *>(m_customData.constData() + byteIdx);
    }
    double t = m_sampleRate > 0 ? static_cast<double>(idx) / m_sampleRate : 0;
    double phase = 2.0 * M_PI * m_frequency * t;
    double val = 0.0;
    switch (m_waveform) {
    case Sine: val = qSin(phase); break;
    case Square: val = qSin(phase) >= 0 ? 1.0 : -1.0; break;
    case Triangle: val = 2.0 * qAbs(2.0 * (phase / (2*M_PI) - qFloor(phase/(2*M_PI) + 0.5))) - 1.0; break;
    case Sawtooth: val = 2.0 * (phase / (2*M_PI) - qFloor(phase / (2*M_PI) + 0.5)); break;
    case Noise: val = 2.0 * QRandomGenerator::global()->generateDouble() - 1.0; break;
    case Custom: break;
    }
    return m_amplitude * val + m_offset;
}
