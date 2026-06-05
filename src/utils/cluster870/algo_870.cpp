/**
 * @file algo_870.cpp
 * @brief Algorithm module 870
 */
#include "cluster870/algo_870.h"
QVector<double> algo_870::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
