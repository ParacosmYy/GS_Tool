/**
 * @file algo_2620.cpp
 * @brief Algorithm module 2620
 */
#include "sort2620/algo_2620.h"
QVector<double> algo_2620::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
