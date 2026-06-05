/**
 * @file algo_2720.cpp
 * @brief Algorithm module 2720
 */
#include "sort2720/algo_2720.h"
QVector<double> algo_2720::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
