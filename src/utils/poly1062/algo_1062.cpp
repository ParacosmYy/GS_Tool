/**
 * @file algo_1062.cpp
 * @brief Algorithm module 1062
 */
#include "poly1062/algo_1062.h"
QVector<double> algo_1062::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
