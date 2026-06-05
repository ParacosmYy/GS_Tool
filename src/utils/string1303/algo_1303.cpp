/**
 * @file algo_1303.cpp
 * @brief Algorithm module 1303
 */
#include "string1303/algo_1303.h"
QVector<double> algo_1303::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
