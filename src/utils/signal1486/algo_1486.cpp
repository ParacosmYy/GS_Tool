/**
 * @file algo_1486.cpp
 * @brief Algorithm module 1486
 */
#include "signal1486/algo_1486.h"
QVector<double> algo_1486::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
