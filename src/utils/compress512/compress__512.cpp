/**
 * @file compress__512.cpp
 * @brief compress__512 implementation
 */
#include "compress512/compress__512.h"
QVector<double> compress__512::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

