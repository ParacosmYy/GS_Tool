/**
 * @file algo_852.cpp
 * @brief Algorithm module 852
 */
#include "compress852/algo_852.h"
QVector<double> algo_852::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
