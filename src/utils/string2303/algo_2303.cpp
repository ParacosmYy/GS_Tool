/**
 * @file algo_2303.cpp
 * @brief Algorithm module 2303
 */
#include "string2303/algo_2303.h"
QVector<double> algo_2303::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
