/**
 * @file algo_2781.cpp
 * @brief Algorithm module 2781
 */
#include "interp2781/algo_2781.h"
QVector<double> algo_2781::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
