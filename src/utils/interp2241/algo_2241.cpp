/**
 * @file algo_2241.cpp
 * @brief Algorithm module 2241
 */
#include "interp2241/algo_2241.h"
QVector<double> algo_2241::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
