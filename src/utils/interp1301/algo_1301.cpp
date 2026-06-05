/**
 * @file algo_1301.cpp
 * @brief Algorithm module 1301
 */
#include "interp1301/algo_1301.h"
QVector<double> algo_1301::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
