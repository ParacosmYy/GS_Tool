/**
 * @file algo_920.cpp
 * @brief Algorithm module 920
 */
#include "sort920/algo_920.h"
QVector<double> algo_920::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
