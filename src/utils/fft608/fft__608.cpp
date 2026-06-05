/**
 * @file fft__608.cpp
 * @brief fft__608 implementation
 */
#include "fft608/fft__608.h"
QVector<double> fft__608::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

