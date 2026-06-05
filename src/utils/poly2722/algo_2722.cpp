/**
 * @file algo_2722.cpp
 * @brief Algorithm module 2722
 */
#include "poly2722/algo_2722.h"
QVector<double> algo_2722::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
