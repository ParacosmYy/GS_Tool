#include "utils/notch/NotchFilter.h"
#include <QElapsedTimer>
#include <cmath>
NotchFilter::NotchFilter(QObject* p) : QObject(p), m_b0(1), m_b1(0), m_b2(1), m_a1(0), m_a2(0),
    m_x1(0), m_x2(0), m_y1(0), m_y2(0), m_freq(0), m_timeSum(0.0) {}
void NotchFilter::design(double freq, double qFactor, double sampleRate) {
    QElapsedTimer t; t.start(); m_freq = freq;
    double w0 = 2.0*M_PI*freq/sampleRate, alpha = std::sin(w0)/(2.0*qFactor);
    m_b0 = 1.0; m_b1 = -2.0*std::cos(w0); m_b2 = 1.0;
    double a0 = 1.0 + alpha;
    m_a1 = -2.0*std::cos(w0)/a0; m_a2 = (1.0-alpha)/a0;
    m_b0 /= a0; m_b1 /= a0; m_b2 /= a0;
    m_x1=m_x2=m_y1=m_y2=0.0;
    m_stats.totalDesigns++; m_timeSum += t.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum/(m_stats.totalDesigns+m_stats.totalApplications);
    emit designCompleted(freq, qFactor);
}
QVector<double> NotchFilter::apply(const QVector<double>& sig) {
    QElapsedTimer t; t.start();
    QVector<double> out(sig.size());
    for (int i = 0; i < sig.size(); ++i) out[i] = processSample(sig[i]);
    m_stats.totalApplications++; m_timeSum += t.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum/(m_stats.totalDesigns+m_stats.totalApplications);
    emit applicationCompleted(sig.size()); return out;
}
double NotchFilter::processSample(double s) {
    double y = m_b0*s + m_b1*m_x1 + m_b2*m_x2 - m_a1*m_y1 - m_a2*m_y2;
    m_x2=m_x1; m_x1=s; m_y2=m_y1; m_y1=y; return y;
}
void NotchFilter::resetState() { m_x1=m_x2=m_y1=m_y2=0.0; }
void NotchFilter::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
