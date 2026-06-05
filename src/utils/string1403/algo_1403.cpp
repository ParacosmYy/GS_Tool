/**
 * @file algo_1403.cpp
 * @brief Algorithm module 1403
 */
#include "string1403/algo_1403.h"
QVector<double> algo_1403::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
