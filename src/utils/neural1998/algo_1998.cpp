/**
 * @file algo_1998.cpp
 * @brief Algorithm module 1998
 */
#include "neural1998/algo_1998.h"
QVector<double> algo_1998::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
