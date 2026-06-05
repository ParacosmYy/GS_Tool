/**
 * @file algo_1721.cpp
 * @brief Algorithm module 1721
 */
#include "interp1721/algo_1721.h"
QVector<double> algo_1721::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
