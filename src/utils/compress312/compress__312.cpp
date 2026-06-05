/**
 * @file compress__312.cpp
 * @brief compress__312 implementation
 */
#include "compress312/compress__312.h"
QVector<double> compress__312::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

