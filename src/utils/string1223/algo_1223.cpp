/**
 * @file algo_1223.cpp
 * @brief Algorithm module 1223
 */
#include "string1223/algo_1223.h"
QVector<double> algo_1223::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
