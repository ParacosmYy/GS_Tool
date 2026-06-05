/**
 * @file algo_1287.cpp
 * @brief Algorithm module 1287
 */
#include "dsp1287/algo_1287.h"
QVector<double> algo_1287::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
