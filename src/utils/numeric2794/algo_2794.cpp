/**
 * @file algo_2794.cpp
 * @brief Algorithm module 2794
 */
#include "numeric2794/algo_2794.h"
QVector<double> algo_2794::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
