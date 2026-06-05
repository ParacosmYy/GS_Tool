/**
 * @file algo_2003.cpp
 * @brief Algorithm module 2003
 */
#include "string2003/algo_2003.h"
QVector<double> algo_2003::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
