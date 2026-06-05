/**
 * @file algo_2466.cpp
 * @brief Algorithm module 2466
 */
#include "signal2466/algo_2466.h"
QVector<double> algo_2466::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
