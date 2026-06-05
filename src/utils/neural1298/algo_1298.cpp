/**
 * @file algo_1298.cpp
 * @brief Algorithm module 1298
 */
#include "neural1298/algo_1298.h"
QVector<double> algo_1298::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
