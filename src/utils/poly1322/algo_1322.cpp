/**
 * @file algo_1322.cpp
 * @brief Algorithm module 1322
 */
#include "poly1322/algo_1322.h"
QVector<double> algo_1322::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
