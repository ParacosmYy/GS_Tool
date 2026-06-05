/**
 * @file compress__412.cpp
 * @brief compress__412 implementation
 */
#include "compress412/compress__412.h"
QVector<double> compress__412::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

