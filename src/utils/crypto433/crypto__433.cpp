/**
 * @file crypto__433.cpp
 * @brief crypto__433 implementation
 */
#include "crypto433/crypto__433.h"
QVector<double> crypto__433::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

