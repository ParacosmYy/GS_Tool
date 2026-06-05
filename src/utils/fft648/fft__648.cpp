/**
 * @file fft__648.cpp
 * @brief fft__648 implementation
 */
#include "fft648/fft__648.h"
QVector<double> fft__648::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

