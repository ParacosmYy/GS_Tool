/**
 * @file algo_2081.cpp
 * @brief Algorithm module 2081
 */
#include "interp2081/algo_2081.h"
QVector<double> algo_2081::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
