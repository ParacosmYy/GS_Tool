/**
 * @file algo_1234.cpp
 * @brief Algorithm module 1234
 */
#include "numeric1234/algo_1234.h"
QVector<double> algo_1234::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
