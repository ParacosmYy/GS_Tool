/**
 * @file algo_1046.cpp
 * @brief Algorithm module 1046
 */
#include "signal1046/algo_1046.h"
QVector<double> algo_1046::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
