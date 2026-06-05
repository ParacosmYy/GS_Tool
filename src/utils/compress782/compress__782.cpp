/**
 * @file compress__782.cpp
 * @brief compress__782 implementation
 */
#include "compress782/compress__782.h"
QVector<double> compress__782::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

