/**
 * @file algo_1580.cpp
 * @brief Algorithm module 1580
 */
#include "sort1580/algo_1580.h"
QVector<double> algo_1580::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
