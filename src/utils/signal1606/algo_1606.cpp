/**
 * @file algo_1606.cpp
 * @brief Algorithm module 1606
 */
#include "signal1606/algo_1606.h"
QVector<double> algo_1606::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
