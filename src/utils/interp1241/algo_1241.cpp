/**
 * @file algo_1241.cpp
 * @brief Algorithm module 1241
 */
#include "interp1241/algo_1241.h"
QVector<double> algo_1241::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
