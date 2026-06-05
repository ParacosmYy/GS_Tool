/**
 * @file algo_1603.cpp
 * @brief Algorithm module 1603
 */
#include "string1603/algo_1603.h"
QVector<double> algo_1603::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
