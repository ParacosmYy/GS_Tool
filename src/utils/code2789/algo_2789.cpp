/**
 * @file algo_2789.cpp
 * @brief Algorithm module 2789
 */
#include "code2789/algo_2789.h"
QVector<double> algo_2789::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
