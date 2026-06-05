/**
 * @file fft__728.cpp
 * @brief fft__728 implementation
 */
#include "fft728/fft__728.h"
QVector<double> fft__728::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

