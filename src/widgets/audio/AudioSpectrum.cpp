#include "widgets/audio/AudioSpectrum.h"
#include <QPainter>
#include <QResizeEvent>
#include <QtMath>

AudioSpectrum::AudioSpectrum(QWidget *parent) : QWidget(parent) {
    setObjectName("AudioSpectrum");
    m_magnitudes.resize(m_barCount);
    m_smoothed.resize(m_barCount);
}
AudioSpectrum::~AudioSpectrum() = default;
void AudioSpectrum::setSampleRate(int r) { m_sampleRate = r; }
void AudioSpectrum::setFftSize(int s) { m_fftSize = s; }
void AudioSpectrum::setDbRange(double min, double max) { m_minDb = min; m_maxDb = max; }
void AudioSpectrum::setBarCount(int b) { m_barCount = b; m_magnitudes.resize(b); m_smoothed.resize(b); }
void AudioSpectrum::setSmoothFactor(double f) { m_smoothFactor = f; }
int AudioSpectrum::sampleRate() const { return m_sampleRate; }
int AudioSpectrum::fftSize() const { return m_fftSize; }

void AudioSpectrum::feedData(const QByteArray &pcmData) {
    m_buffer.append(pcmData);
    if (m_buffer.size() >= m_fftSize * 2) {
        processFft();
        m_buffer.remove(0, m_fftSize);
    }
}

void AudioSpectrum::processFft() {
    int binsPerBar = m_fftSize / 2 / m_barCount;
    for (int i = 0; i < m_barCount; ++i) {
        double mag = 0;
        for (int j = 0; j < binsPerBar; ++j) {
            int idx = i * binsPerBar + j;
            if (idx * 2 + 1 < m_buffer.size()) {
                qint16 sample = static_cast<qint16>((static_cast<uint8_t>(m_buffer[idx*2+1]) << 8) | static_cast<uint8_t>(m_buffer[idx*2]));
                mag += sample * sample;
            }
        }
        mag = qSqrt(mag / binsPerBar);
        double db = 20.0 * qLn(qMax(mag, 1.0)) / qLn(10.0);
        db = qBound(m_minDb, db, m_maxDb);
        double norm = (db - m_minDb) / (m_maxDb - m_minDb);
        m_smoothed[i] = m_smoothFactor * m_smoothed[i] + (1.0 - m_smoothFactor) * norm;
        m_magnitudes[i] = norm;
    }
    emit spectrumUpdated(m_magnitudes);
    int peakBar = 0; double peakVal = 0;
    for (int i = 0; i < m_barCount; ++i) if (m_smoothed[i] > peakVal) { peakVal = m_smoothed[i]; peakBar = i; }
    double freq = static_cast<double>(peakBar * binsPerBar) * m_sampleRate / m_fftSize;
    emit peakFrequencyChanged(freq);
    update();
}

void AudioSpectrum::paintEvent(QPaintEvent *) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    int w = width(), h = height();
    p.fillRect(rect(), Qt::transparent);
    double barW = static_cast<double>(w) / m_barCount * 0.8;
    double gap = static_cast<double>(w) / m_barCount * 0.2;
    for (int i = 0; i < m_barCount; ++i) {
        double barH = m_smoothed[i] * h;
        QColor c = QColor::fromHsvF(static_cast<double>(i) / m_barCount * 0.7, 0.8, 0.9);
        p.setBrush(c); p.setPen(Qt::NoPen);
        p.drawRoundedRect(static_cast<double>(i) * (barW + gap), h - barH, barW, barH, 2, 2);
    }
}
void AudioSpectrum::resizeEvent(QResizeEvent *e) { QWidget::resizeEvent(e); }
