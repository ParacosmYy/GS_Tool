/**
 * @file algo_1261.cpp
 * @brief Algorithm module 1261
 */
#include "interp1261/algo_1261.h"
QVector<double> algo_1261::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
