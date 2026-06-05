/**
 * @file algo_1278.cpp
 * @brief Algorithm module 1278
 */
#include "neural1278/algo_1278.h"
QVector<double> algo_1278::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
