/**
 * @file fft__398.cpp
 * @brief fft__398 implementation
 */
#include "fft398/fft__398.h"
QVector<double> fft__398::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

