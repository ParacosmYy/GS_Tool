/**
 * @file algo_851.cpp
 * @brief Algorithm module 851
 */
#include "tree851/algo_851.h"
QVector<double> algo_851::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
