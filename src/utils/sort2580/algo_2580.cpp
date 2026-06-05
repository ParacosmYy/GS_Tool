/**
 * @file algo_2580.cpp
 * @brief Algorithm module 2580
 */
#include "sort2580/algo_2580.h"
QVector<double> algo_2580::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
