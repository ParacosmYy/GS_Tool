/**
 * @file algo_1123.cpp
 * @brief Algorithm module 1123
 */
#include "string1123/algo_1123.h"
QVector<double> algo_1123::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
