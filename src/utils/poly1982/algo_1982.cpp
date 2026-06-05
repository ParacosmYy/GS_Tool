/**
 * @file algo_1982.cpp
 * @brief Algorithm module 1982
 */
#include "poly1982/algo_1982.h"
QVector<double> algo_1982::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
