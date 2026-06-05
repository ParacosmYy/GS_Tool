/**
 * @file algo_2394.cpp
 * @brief Algorithm module 2394
 */
#include "numeric2394/algo_2394.h"
QVector<double> algo_2394::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
