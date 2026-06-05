/**
 * @file algo_882.cpp
 * @brief Algorithm module 882
 */
#include "poly882/algo_882.h"
QVector<double> algo_882::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
