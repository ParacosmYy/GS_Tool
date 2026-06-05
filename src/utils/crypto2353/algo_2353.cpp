/**
 * @file algo_2353.cpp
 * @brief Algorithm module 2353
 */
#include "crypto2353/algo_2353.h"
QVector<double> algo_2353::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
