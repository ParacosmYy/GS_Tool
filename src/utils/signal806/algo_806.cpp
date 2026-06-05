/**
 * @file algo_806.cpp
 * @brief Algorithm module 806
 */
#include "signal806/algo_806.h"
QVector<double> algo_806::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
