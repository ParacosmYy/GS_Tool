/**
 * @file algo_2626.cpp
 * @brief Algorithm module 2626
 */
#include "signal2626/algo_2626.h"
QVector<double> algo_2626::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
