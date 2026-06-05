/**
 * @file compress__662.cpp
 * @brief compress__662 implementation
 */
#include "compress662/compress__662.h"
QVector<double> compress__662::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

