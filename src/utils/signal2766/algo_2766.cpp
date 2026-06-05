/**
 * @file algo_2766.cpp
 * @brief Algorithm module 2766
 */
#include "signal2766/algo_2766.h"
QVector<double> algo_2766::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
