/**
 * @file algo_2740.cpp
 * @brief Algorithm module 2740
 */
#include "sort2740/algo_2740.h"
QVector<double> algo_2740::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
