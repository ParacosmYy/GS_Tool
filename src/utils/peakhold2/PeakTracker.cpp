#include "utils/peakhold2/PeakTracker.h"
#include <QElapsedTimer>
#include <algorithm>

PeakTracker::PeakTracker(int windowSize, double decayFactor, QObject* parent)
    : QObject(parent), m_windowSize(qMax(3, windowSize)),
      m_decayFactor(qBound(0.0, decayFactor, 1.0)), m_writeIdx(0) {}

void PeakTracker::update(double value) {
    QElapsedTimer timer; timer.start();

    /* 衰减已有值 */
    if (!m_buffer.isEmpty()) {
        m_buffer[m_writeIdx] = value;
    } else {
        m_buffer.resize(m_windowSize, 0.0);
        m_buffer[0] = value;
    }
    m_writeIdx = (m_writeIdx + 1) % m_windowSize;

    /* 应用衰减 */
    for (auto& v : m_buffer) v *= m_decayFactor;
    m_buffer[(m_writeIdx - 1 + m_windowSize) % m_windowSize] = value;

    detectPeaks();

    m_timeSum += static_cast<double>(timer.elapsed());
    ++m_stats.totalUpdates;
    m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_stats.totalUpdates);
}

QVector<PeakTracker::Peak> PeakTracker::getPeaks() const { return m_peaks; }

void PeakTracker::reset() {
    m_buffer.clear();
    m_writeIdx = 0;
    m_peaks.clear();
    m_currentPeak = Peak{};
}

void PeakTracker::detectPeaks() {
    m_peaks.clear();
    int n = m_buffer.size();
    if (n < 3) return;

    for (int i = 1; i < n - 1; ++i) {
        if (m_buffer[i] > m_buffer[i - 1] && m_buffer[i] > m_buffer[i + 1]) {
            Peak p;
            p.value = m_buffer[i];
            p.position = i;
            m_peaks.append(p);
        }
    }

    std::sort(m_peaks.begin(), m_peaks.end(),
              [](const Peak& a, const Peak& b) { return a.value > b.value; });

    if (!m_peaks.isEmpty()) {
        if (m_peaks[0].value > m_currentPeak.value * m_decayFactor) {
            m_currentPeak = m_peaks[0];
            ++m_stats.totalPeaks;
            emit peakDetected(m_currentPeak.value, m_currentPeak.position);
        }
    }
}

void PeakTracker::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
