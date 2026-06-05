/**
 * @file compress__332.cpp
 * @brief compress__332 implementation
 */
#include "compress332/compress__332.h"
QVector<double> compress__332::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

