/**
 * @file algo_2123.cpp
 * @brief Algorithm module 2123
 */
#include "string2123/algo_2123.h"
QVector<double> algo_2123::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
