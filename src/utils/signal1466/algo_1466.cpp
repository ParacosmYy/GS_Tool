/**
 * @file algo_1466.cpp
 * @brief Algorithm module 1466
 */
#include "signal1466/algo_1466.h"
QVector<double> algo_1466::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
