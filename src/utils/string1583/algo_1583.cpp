/**
 * @file algo_1583.cpp
 * @brief Algorithm module 1583
 */
#include "string1583/algo_1583.h"
QVector<double> algo_1583::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
