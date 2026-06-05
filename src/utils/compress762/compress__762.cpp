/**
 * @file compress__762.cpp
 * @brief compress__762 implementation
 */
#include "compress762/compress__762.h"
QVector<double> compress__762::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

