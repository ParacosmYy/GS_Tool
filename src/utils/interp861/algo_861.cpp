/**
 * @file algo_861.cpp
 * @brief Algorithm module 861
 */
#include "interp861/algo_861.h"
QVector<double> algo_861::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
