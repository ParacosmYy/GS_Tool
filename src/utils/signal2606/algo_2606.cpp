/**
 * @file algo_2606.cpp
 * @brief Algorithm module 2606
 */
#include "signal2606/algo_2606.h"
QVector<double> algo_2606::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
