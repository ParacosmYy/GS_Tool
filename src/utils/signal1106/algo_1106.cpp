/**
 * @file algo_1106.cpp
 * @brief Algorithm module 1106
 */
#include "signal1106/algo_1106.h"
QVector<double> algo_1106::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
