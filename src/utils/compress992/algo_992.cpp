/**
 * @file algo_992.cpp
 * @brief Algorithm module 992
 */
#include "compress992/algo_992.h"
QVector<double> algo_992::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
