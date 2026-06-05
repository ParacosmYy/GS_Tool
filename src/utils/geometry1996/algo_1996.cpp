/**
 * @file algo_1996.cpp
 * @brief Algorithm module 1996
 */
#include "geometry1996/algo_1996.h"
QVector<double> algo_1996::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
