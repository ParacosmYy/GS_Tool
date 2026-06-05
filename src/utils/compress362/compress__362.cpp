/**
 * @file compress__362.cpp
 * @brief compress__362 implementation
 */
#include "compress362/compress__362.h"
QVector<double> compress__362::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

