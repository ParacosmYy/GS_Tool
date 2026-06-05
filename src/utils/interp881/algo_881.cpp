/**
 * @file algo_881.cpp
 * @brief Algorithm module 881
 */
#include "interp881/algo_881.h"
QVector<double> algo_881::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
