/**
 * @file algo_952.cpp
 * @brief Algorithm module 952
 */
#include "compress952/algo_952.h"
QVector<double> algo_952::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
