/**
 * @file algo_1782.cpp
 * @brief Algorithm module 1782
 */
#include "poly1782/algo_1782.h"
QVector<double> algo_1782::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
