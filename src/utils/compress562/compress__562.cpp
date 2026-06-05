/**
 * @file compress__562.cpp
 * @brief compress__562 implementation
 */
#include "compress562/compress__562.h"
QVector<double> compress__562::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

