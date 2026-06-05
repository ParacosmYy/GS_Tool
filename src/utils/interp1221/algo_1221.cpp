/**
 * @file algo_1221.cpp
 * @brief Algorithm module 1221
 */
#include "interp1221/algo_1221.h"
QVector<double> algo_1221::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
