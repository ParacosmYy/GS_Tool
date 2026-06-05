/**
 * @file algo_2006.cpp
 * @brief Algorithm module 2006
 */
#include "signal2006/algo_2006.h"
QVector<double> algo_2006::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
