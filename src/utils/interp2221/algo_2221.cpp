/**
 * @file algo_2221.cpp
 * @brief Algorithm module 2221
 */
#include "interp2221/algo_2221.h"
QVector<double> algo_2221::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
