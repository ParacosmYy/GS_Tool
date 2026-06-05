/**
 * @file algo_2414.cpp
 * @brief Algorithm module 2414
 */
#include "numeric2414/algo_2414.h"
QVector<double> algo_2414::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
